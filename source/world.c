#include <string.h>
#include <3ds.h>

#include "gs.h"
#include "world.h"
#include "sdnoise.h"
#include "text.h"

#define pushFace(l, s, f) ((l)[(s)++]=f)
#define popFace(l, s) ((s)?(&((l)[--(s)])):NULL)

#define CHUNKPOOL_ALLOCSIZE ((WORLD_SIZE*WORLD_SIZE)/2)

worldChunk_s* chunkPool;

extern u32 debugValue[128];

void initChunkPool(void)
{
	chunkPool=NULL;
}

void allocatePoolChunks(void)
{
	worldChunk_s* newChunks=malloc(sizeof(worldChunk_s)*CHUNKPOOL_ALLOCSIZE);
	int i; for(i=0;i<CHUNKPOOL_ALLOCSIZE-1;i++)newChunks[i].next=&newChunks[i+1];
	newChunks[CHUNKPOOL_ALLOCSIZE-1].next=chunkPool;
	chunkPool=newChunks;
}

worldChunk_s* getNewChunk(void)
{
	if(!chunkPool)allocatePoolChunks();
	worldChunk_s* wch=chunkPool;
	chunkPool=wch->next;
	wch->next=NULL;
	return wch;
}

void freeChunk(worldChunk_s* wch)
{
	if(!wch)return;
	wch->next=chunkPool;
	chunkPool=wch;
}

void initWorldCluster(worldCluster_s* wcl, vect3Di_s pos)
{
	if(!wcl)return;

	memset(wcl->data, 0x00, CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE);
	wcl->position=pos;
	gsVboInit(&wcl->vbo);
	wcl->generated=false;
	wcl->busy=false;
}

vect3Df_s clusterCoordToWorld(vect3Di_s v)
{
	return (vect3Df_s){v.x*CLUSTER_SIZE, v.y*CLUSTER_SIZE, v.z*CLUSTER_SIZE};
}

void drawWorldCluster(worldCluster_s* wcl)
{
	if(!wcl || !wcl->generated || wcl->busy)return;
	vect3Df_s v=clusterCoordToWorld(wcl->position);

	gsVboDraw(&wcl->vbo);
}

void generateWorldClusterGeometry(worldCluster_s* wcl, world_s* w)
{
	if(!wcl)return;
	if(wcl->generated)gsVboDestroy(&wcl->vbo);

	wcl->generated=false;

	//first, we go through the whole cluster to generate a "list" of faces
	static blockFace_s faceList[4096]; //TODO : calculate real max
	int faceListSize=0;

	memset(faceList, 0x00, 4096*sizeof(blockFace_s));

	//TODO : optimize world block accesses
	const vect3Di_s p = vmuli(wcl->position, CLUSTER_SIZE);
	int i, j, k;
	for(i=0; i<CLUSTER_SIZE; i++)
	{
		for(j=0; j<CLUSTER_SIZE; j++)
		{
			for(k=0; k<CLUSTER_SIZE; k++)
			{
				const u8 cb=wcl->data[i][j][k];
				if(i>=1 && i<CLUSTER_SIZE-1 && j>=1 && j<CLUSTER_SIZE-1 && k>=1 && k<CLUSTER_SIZE-1)
				{
					if(blockShouldBeFace(cb, wcl->data[i+1][j][k])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_PX, vect3Di(i,j,k)));
					if(blockShouldBeFace(cb, wcl->data[i-1][j][k])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_MX, vect3Di(i,j,k)));
					if(blockShouldBeFace(cb, wcl->data[i][j+1][k])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_PY, vect3Di(i,j,k)));
					if(blockShouldBeFace(cb, wcl->data[i][j-1][k])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_MY, vect3Di(i,j,k)));
					if(blockShouldBeFace(cb, wcl->data[i][j][k+1])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_PZ, vect3Di(i,j,k)));
					if(blockShouldBeFace(cb, wcl->data[i][j][k-1])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_MZ, vect3Di(i,j,k)));
				}else if(w){
					if(blockShouldBeFace(cb, getWorldBlock(w, vaddi(p, vect3Di(i+1, j, k))))>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_PX, vect3Di(i,j,k)));
					if(blockShouldBeFace(cb, getWorldBlock(w, vaddi(p, vect3Di(i-1, j, k))))>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_MX, vect3Di(i,j,k)));
					if(blockShouldBeFace(cb, getWorldBlock(w, vaddi(p, vect3Di(i, j+1, k))))>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_PY, vect3Di(i,j,k)));
					if(blockShouldBeFace(cb, getWorldBlock(w, vaddi(p, vect3Di(i, j-1, k))))>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_MY, vect3Di(i,j,k)));
					if(blockShouldBeFace(cb, getWorldBlock(w, vaddi(p, vect3Di(i, j, k+1))))>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_PZ, vect3Di(i,j,k)));
					if(blockShouldBeFace(cb, getWorldBlock(w, vaddi(p, vect3Di(i, j, k-1))))>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_MZ, vect3Di(i,j,k)));
				}
			}
		}
	}

	//then, we set up VBO size to create the VBO
	const u32 size=faceListSize*FACE_VBO_SIZE;
	vect3Df_s off=clusterCoordToWorld(wcl->position);

	if(!gsVboCreate(&wcl->vbo, size))
	{
		//and if that succeeds, we transfer all those faces to the VBO !
		blockFace_s* bf;
		while((bf=popFace(faceList, faceListSize)))
		{
			blockGenerateFaceGeometry(bf, &wcl->vbo, off);
		}

		gsVboFlushData(&wcl->vbo);
		wcl->generated=true;
	}
}

int getWorldElevation(vect3Di_s p)
{
	return (int)(sdnoise2(((float)p.x)/(CLUSTER_SIZE*4), ((float)p.z)/(CLUSTER_SIZE*4), NULL, NULL)*CLUSTER_SIZE)+(CHUNK_HEIGHT*CLUSTER_SIZE/2);
}

void generateWorldClusterData(worldCluster_s* wcl)
{
	if(!wcl)return;
	if(wcl->generated)gsVboDestroy(&wcl->vbo);

	//TEMP
	int i, j, k;
	for(i=0; i<CLUSTER_SIZE; i++)
	{
		for(k=0; k<CLUSTER_SIZE; k++)
		{
			//TEMP
			const vect3Di_s p=vect3Di(i+wcl->position.x*CLUSTER_SIZE, wcl->position.y*CLUSTER_SIZE, k+wcl->position.z*CLUSTER_SIZE);
			int height=getWorldElevation(p);
			for(j=0; j<CLUSTER_SIZE; j++)
			{
				if(p.y+j < height)wcl->data[i][j][k]=BLOCK_GRASS;
				else wcl->data[i][j][k]=BLOCK_AIR;
			}
		}
	}
}

s16 getWorldClusterBlock(worldCluster_s* wcl, vect3Di_s p)
{
	if(!wcl || wcl->busy)return -1;
	if(p.x<0 || p.y<0 || p.z<0)return -1;
	if(p.x>=CLUSTER_SIZE || p.y>=CLUSTER_SIZE || p.z>=CLUSTER_SIZE)return -1;

	return wcl->data[p.x][p.y][p.z];
}

void initWorldChunk(worldChunk_s* wch, vect3Di_s pos)
{
	if(!wch)return;

	int j; for(j=0; j<CHUNK_HEIGHT; j++)initWorldCluster(&wch->data[j], vect3Di(pos.x, j, pos.z));
	wch->position=pos;
}

//TEMP ?
void generateWorldChunkData(worldChunk_s* wch)
{
	if(!wch)return;

	int k; for(k=0; k<CHUNK_HEIGHT; k++)generateWorldClusterData(&wch->data[k]);
}

//TEMP ?
void generateWorldChunkGeometry(worldChunk_s* wch, world_s* w)
{
	if(!wch)return;

	int k; for(k=0; k<CHUNK_HEIGHT; k++)generateWorldClusterGeometry(&wch->data[k], w);
}

void drawWorldChunk(worldChunk_s* wch, camera_s* c)
{
	if(!wch)return;

	//baseline culling
	if(!aabbInCameraFrustum(c, clusterCoordToWorld(wch->position), vect3Df(CLUSTER_SIZE,CLUSTER_SIZE*CHUNK_HEIGHT,CLUSTER_SIZE)))return;
	debugValue[0]++;
	int k; for(k=0; k<CHUNK_HEIGHT; k++)drawWorldCluster(&wch->data[k]);
}

s16 getWorldChunkBlock(worldChunk_s* wc, vect3Di_s p)
{
	if(!wc)return -1;
	if(p.x<0 || p.y<0 || p.z<0)return -1;
	if(p.x>=CLUSTER_SIZE || p.y>=CHUNK_HEIGHT*CLUSTER_SIZE || p.z>=CLUSTER_SIZE)return -1;

	return getWorldClusterBlock(&wc->data[p.y/CLUSTER_SIZE], vect3Di(p.x, p.y%CLUSTER_SIZE, p.z));
}

void initWorld(world_s* w)
{
	if(!w)return;

	int i, j;
	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			w->data[i][j]=NULL;
		}
	}

	w->position=vect3Di(0,0,0);
}

s16 getWorldBlock(world_s* w, vect3Di_s p)
{
	if(!w)return -1;
	p=vaddi(p,vmuli(w->position,-CLUSTER_SIZE));
	if(p.x<0 || p.y<0 || p.z<0)return -1;
	if(p.x>=WORLD_SIZE*CLUSTER_SIZE || p.y>=CHUNK_HEIGHT*CLUSTER_SIZE || p.z>=WORLD_SIZE*CLUSTER_SIZE)return -1;

	return getWorldChunkBlock(w->data[p.x/CLUSTER_SIZE][p.z/CLUSTER_SIZE], vect3Di(p.x%CLUSTER_SIZE, p.y, p.z%CLUSTER_SIZE));
}

void updateWorld(world_s* w)
{
	if(!w)return;

	int i, j;
	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			//TEMP, naive generation
			if(!w->data[i][j])
			{
				w->data[i][j]=getNewChunk();
				if(w->data[i][j])
				{
					initWorldChunk(w->data[i][j], vect3Di(i+w->position.x,0,j+w->position.y));
					generateWorldChunkData(w->data[i][j]);
					generateWorldChunkGeometry(w->data[i][j], w); //will miss faces, but this is TEMP anyway
				}
			}
		}
	}
}

void translateWorld(world_s* w, vect3Di_s v)
{
	if(!w)return;

	//waaaaay suboptimal but this won't get called often so who cares
	static worldChunk_s* tmpData[WORLD_SIZE][WORLD_SIZE];
	memcpy(tmpData, w->data, sizeof(tmpData));
	memset(w->data, 0x00, sizeof(tmpData));

	int i, j;
	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			if(i-v.x >= 0 && i-v.x < WORLD_SIZE && j-v.y >= 0 && j-v.y < WORLD_SIZE)w->data[i-v.x][j-v.y]=tmpData[i][j];
			else{freeChunk(tmpData[i][j]); tmpData[i][j]=NULL;}
		}
	}

	w->position=vaddi(w->position,v);
}

void drawWorld(world_s* w, camera_s* c)
{
	if(!w)return;

	u64 val=svcGetSystemTick();
	int i, j;
	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			drawWorldChunk(w->data[i][j], c);
		}
	}
	debugValue[1]=(u32)(svcGetSystemTick()-val);
}
