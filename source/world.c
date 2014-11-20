#include <string.h>
#include <3ds.h>

#include "gs.h"
#include "world.h"
#include "dispatcher.h"
#include "sdnoise.h"
#include "text.h"
#include "configuration.h"
#include "generation.h"

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
	wch->world=NULL;
	return wch;
}

bool isChunkBusy(worldChunk_s* wch)
{
	if(!wch)return false;
	int i; for(i=0;i<CHUNK_HEIGHT;i++)if(wch->data[i].status&WCL_BUSY)return true;
	return false;
}

void fixChunk(worldChunk_s* wch)
{
	if(!wch || !wch->next || isChunkBusy(wch))return;
	wch->next=NULL;
	freeChunk(wch);
}

void freeCluster(worldCluster_s* wcl)
{
	if(!wcl || wcl->status&WCL_BUSY)return;
	if(!(wcl->status&WCL_GEOM_UNAVAILABLE))gsVboDestroy(&wcl->vbo);
}

void freeChunk(worldChunk_s* wch)
{
	if(!wch)return;
	if(isChunkBusy(wch) || wch->modified)
	{
		//put it aside if it might still have producers working on it or if we need to save it
		wch->next=(void*)0xFFFFFFFF;
		if(wch->modified)dispatchJob(NULL, createJobDiscardChunk(wch));
		debugValue[7]++;
	}else{
		wch->next=chunkPool;
		chunkPool=wch;
		int i; for(i=0; i<CHUNK_HEIGHT; i++)freeCluster(&wch->data[i]);
	}
}

void initWorldCluster(worldCluster_s* wcl, vect3Di_s pos)
{
	if(!wcl)return;

	memset(wcl->data, 0x00, CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE);
	wcl->position=pos;
	gsVboInit(&wcl->vbo);
	wcl->status=WCL_DATA_UNAVAILABLE|WCL_GEOM_UNAVAILABLE;
	wcl->directions=0;
}

vect3Df_s clusterCoordToWorld(vect3Di_s v)
{
	return (vect3Df_s){v.x*CLUSTER_SIZE, v.y*CLUSTER_SIZE, v.z*CLUSTER_SIZE};
}

void drawWorldCluster(world_s* w, worldChunk_s* wch, worldCluster_s* wcl)
{
	if(!wcl || !wch || !w)return;
	if(wcl->status&WCL_GEOM_UNAVAILABLE)
	{
		if(!(wcl->status&WCL_BUSY) && !(wcl->status&WCL_DATA_UNAVAILABLE))dispatchJob(NULL, createJobGenerateClusterGeometry(wcl, wch, w));
		return;
	}
	gsVboDraw(&wcl->vbo);
}

void generateWorldClusterGeometry(worldCluster_s* wcl, world_s* w, blockFace_s* tmpBuffer, int tmpBufferSize)
{
	if(!wcl)return;
	if(!(wcl->status&WCL_GEOM_UNAVAILABLE))gsVboDestroy(&wcl->vbo);

	wcl->status|=WCL_GEOM_UNAVAILABLE;
	wcl->directions=0;

	//first, we go through the whole cluster to generate a "list" of faces
	const static int staticFaceBufferSize=4096*sizeof(blockFace_s); //TODO : calculate real max
	static blockFace_s staticFaceList[4096];

	blockFace_s* faceList=staticFaceList;
	int faceBufferSize=staticFaceBufferSize;
	if(tmpBuffer){faceList=tmpBuffer;faceBufferSize=tmpBufferSize;}

	int faceListSize=0;
	memset(faceList, 0x00, faceBufferSize);

	int i, j, k;
	for(i=0; i<CLUSTER_SIZE; i++)
	{
		for(j=0; j<CLUSTER_SIZE; j++)
		{
			for(k=0; k<CLUSTER_SIZE; k++)
			{
				const u8 cb=wcl->data[i][j][k];
				if(i<CLUSTER_SIZE-1 && blockShouldBeFace(cb, wcl->data[i+1][j][k])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_PX, vect3Di(i,j,k)));
				if(i>0              && blockShouldBeFace(cb, wcl->data[i-1][j][k])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_MX, vect3Di(i,j,k)));
				if(j<CLUSTER_SIZE-1 && blockShouldBeFace(cb, wcl->data[i][j+1][k])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_PY, vect3Di(i,j,k)));
				if(j>0              && blockShouldBeFace(cb, wcl->data[i][j-1][k])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_MY, vect3Di(i,j,k)));
				if(k<CLUSTER_SIZE-1 && blockShouldBeFace(cb, wcl->data[i][j][k+1])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_PZ, vect3Di(i,j,k)));
				if(k>0              && blockShouldBeFace(cb, wcl->data[i][j][k-1])>=0) pushFace(faceList, faceListSize, blockFace(cb, FACE_MZ, vect3Di(i,j,k)));
			}
		}
	}

	//check for border faces
	generateWorldAdditionalGeometryList(wcl, w, 0xFF, faceList, faceBufferSize, &faceListSize);

	//then, we set up VBO size to create the VBO
	const u32 size=faceListSize*FACE_VBO_SIZE;
	if(!gsVboCreate(&wcl->vbo, size))buildClusterGeometry(wcl, faceList, faceBufferSize, faceListSize, NULL);
}

int generateWorldAdditionalClusterGeometry(worldCluster_s* wcl, world_s* w, u8 directions, blockFace_s* tmpBuffer, int tmpBufferSize, gsVbo_s* vbo)
{
	if(!wcl || !vbo)return -1;
	if(wcl->status&WCL_GEOM_UNAVAILABLE)return -2; //only do stuff if geometry is available
	int ret=0;

	//first, we go through the whole cluster to generate a "list" of faces
	const static int staticFaceBufferSize=4096*sizeof(blockFace_s); //TODO : calculate real max
	static blockFace_s staticFaceList[4096];

	blockFace_s* faceList=staticFaceList;
	int faceBufferSize=staticFaceBufferSize;
	if(tmpBuffer){faceList=tmpBuffer;faceBufferSize=tmpBufferSize;}

	int faceListSize=0;
	memset(faceList, 0x00, faceBufferSize);

	//check for border faces
	generateWorldAdditionalGeometryList(wcl, w, directions, faceList, faceBufferSize, &faceListSize);

	//then, we set up VBO size to create the VBO
	if(faceListSize>0)
	{
		//grab the vbo data, append new faces to it and be on our way
		const u32 size=wcl->vbo.currentSize+faceListSize*FACE_VBO_SIZE;
		gsVboInit(vbo);
		if(!gsVboCreate(vbo, size))
		{
			gsVboAddData(vbo, wcl->vbo.data, wcl->vbo.currentSize, wcl->vbo.numVertices);
			buildClusterGeometry(wcl, faceList, faceBufferSize, faceListSize, vbo);
			ret=1;
		}
	}

	return ret;
}

void generateWorldAdditionalGeometryList(worldCluster_s* wcl, world_s* w, u8 directions, blockFace_s* faceList, int faceBufferSize, int* faceListSize)
{
	if(!wcl || !faceBufferSize || !faceList || !faceListSize)return;
	directions&=~wcl->directions;
	if(!directions)return;

	const vect3Di_s p = vmuli(wcl->position, CLUSTER_SIZE);
	int i, j, k;

	worldCluster_s* wclp[6] = {NULL,NULL,NULL,NULL,NULL,NULL};

	if(directions&WCL_PX)wclp[0]=getWorldBlockCluster(w, vaddi(p, vect3Di(+CLUSTER_SIZE, 0, 0)));
	if(directions&WCL_MX)wclp[1]=getWorldBlockCluster(w, vaddi(p, vect3Di(-1, 0, 0)));
	if(directions&WCL_PY)wclp[2]=getWorldBlockCluster(w, vaddi(p, vect3Di(0, +CLUSTER_SIZE, 0)));
	if(directions&WCL_MY)wclp[3]=getWorldBlockCluster(w, vaddi(p, vect3Di(0, -1, 0)));
	if(directions&WCL_PZ)wclp[4]=getWorldBlockCluster(w, vaddi(p, vect3Di(0, 0, +CLUSTER_SIZE)));
	if(directions&WCL_MZ)wclp[5]=getWorldBlockCluster(w, vaddi(p, vect3Di(0, 0, -1)));
	for(i=0;i<6;i++)if(wclp[i] && wclp[i]->status&WCL_DATA_UNAVAILABLE)wclp[i]=NULL; else if(wclp[i])wcl->directions|=(1<<i);

	for(j=0; j<CLUSTER_SIZE; j++)
	{
		for(k=0; k<CLUSTER_SIZE; k++)
		{
			u8 cb;
			cb=wcl->data[CLUSTER_SIZE-1][j][k]; if(wclp[0] && blockShouldBeFace(cb, wclp[0]->data[0][j][k])             >=0) pushFace(faceList, (*faceListSize), blockFace(cb, FACE_PX, vect3Di(CLUSTER_SIZE-1,j,k)));
			cb=wcl->data[0][j][k];              if(wclp[1] && blockShouldBeFace(cb, wclp[1]->data[CLUSTER_SIZE-1][j][k])>=0) pushFace(faceList, (*faceListSize), blockFace(cb, FACE_MX, vect3Di(0,j,k)));
			cb=wcl->data[j][CLUSTER_SIZE-1][k]; if(wclp[2] && blockShouldBeFace(cb, wclp[2]->data[j][0][k])             >=0) pushFace(faceList, (*faceListSize), blockFace(cb, FACE_PY, vect3Di(j,CLUSTER_SIZE-1,k)));
			cb=wcl->data[j][0][k];              if(wclp[3] && blockShouldBeFace(cb, wclp[3]->data[j][CLUSTER_SIZE-1][k])>=0) pushFace(faceList, (*faceListSize), blockFace(cb, FACE_MY, vect3Di(j,0,k)));
			cb=wcl->data[j][k][CLUSTER_SIZE-1]; if(wclp[4] && blockShouldBeFace(cb, wclp[4]->data[j][k][0])             >=0) pushFace(faceList, (*faceListSize), blockFace(cb, FACE_PZ, vect3Di(j,k,CLUSTER_SIZE-1)));
			cb=wcl->data[j][k][0];              if(wclp[5] && blockShouldBeFace(cb, wclp[5]->data[j][k][CLUSTER_SIZE-1])>=0) pushFace(faceList, (*faceListSize), blockFace(cb, FACE_MZ, vect3Di(j,k,0)));
		}
	}
}

void buildClusterGeometry(worldCluster_s* wcl, blockFace_s* faceList, int faceBufferSize, int faceListSize, gsVbo_s* vbo)
{
	if(!wcl || !faceList || !faceBufferSize || !faceListSize)return;
	if(!vbo)vbo=&wcl->vbo;

	vect3Df_s off=clusterCoordToWorld(wcl->position);

	//and if that succeeds, we transfer all those faces to the VBO !
	blockFace_s* bf;
	while((bf=popFace(faceList, faceListSize)))
	{
		blockGenerateFaceGeometry(bf, vbo, off);
	}

	gsVboFlushData(vbo);
	wcl->status&=~WCL_GEOM_UNAVAILABLE;
}

void generateWorldClusterData(worldCluster_s* wcl, worldChunk_s* wch)
{
	if(!wcl || !wch)return;
	if(!(wcl->status&WCL_GEOM_UNAVAILABLE)){gsVboDestroy(&wcl->vbo);wcl->status|=WCL_GEOM_UNAVAILABLE;}

	generateWorldCluster(&wch->info, wcl);
}

s16 getWorldClusterBlock(worldCluster_s* wcl, vect3Di_s p)
{
	if(!wcl || (wcl->status&WCL_DATA_UNAVAILABLE))return -1;
	if(p.x<0 || p.y<0 || p.z<0)return -1;
	if(p.x>=CLUSTER_SIZE || p.y>=CLUSTER_SIZE || p.z>=CLUSTER_SIZE)return -1;

	return wcl->data[p.x][p.y][p.z];
}

void initWorldChunk(worldChunk_s* wch, world_s* w, vect3Di_s pos)
{
	if(!wch || !w)return;

	int j; for(j=0; j<CHUNK_HEIGHT; j++)initWorldCluster(&wch->data[j], vect3Di(pos.x, j, pos.z));
	wch->modified=false;
	wch->position=pos;
	wch->world=w;
}

void generateWorldChunkData(worldChunk_s* wch)
{
	if(!wch)return;

	generateWorldChunkInfo(wch, &wch->info);

	int j; for(j=0; j<CHUNK_HEIGHT; j++)generateWorldClusterData(&wch->data[j], wch);
}

void drawWorldChunk(world_s* w, worldChunk_s* wch, camera_s* c)
{
	if(!wch)return;

	//baseline culling
	if(!aabbInCameraFrustum(c, clusterCoordToWorld(wch->position), vect3Df(CLUSTER_SIZE,CLUSTER_SIZE*CHUNK_HEIGHT,CLUSTER_SIZE), 3))return;
	debugValue[0]++;
	int k; for(k=0; k<CHUNK_HEIGHT; k++)drawWorldCluster(w, wch, &wch->data[k]);
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

	static char path[256];
	snprintf(path, 256, "%s/%s", configuration.path, configuration.currentFile);
	initWorldStream(&w->stream, path);

	int i, j;
	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			w->data[i][j]=NULL;
		}
	}

	w->position=vect3Di(-WORLD_SIZE/2,0,-WORLD_SIZE/2);
}

void flushWorld(world_s* w)
{
	if(!w)return;

	int i, j;
	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			if(w->data[i][j]){freeChunk(w->data[i][j]); w->data[i][j]=NULL;}
		}
	}
}

void exitWorld(world_s* w)
{
	if(!w)return;

	exitWorldStream(&w->stream);
}

worldCluster_s* getWorldBlockCluster(world_s* w, vect3Di_s p)
{
	if(!w)return NULL;
	p=vaddi(p,vmuli(w->position,-CLUSTER_SIZE));
	if(p.x<0 || p.y<0 || p.z<0)return NULL;
	if(p.x>=WORLD_SIZE*CLUSTER_SIZE || p.y>=CHUNK_HEIGHT*CLUSTER_SIZE || p.z>=WORLD_SIZE*CLUSTER_SIZE)return NULL;

	worldChunk_s* wch=w->data[p.x/CLUSTER_SIZE][p.z/CLUSTER_SIZE];
	if(!wch || p.y<0 || p.y>=CHUNK_HEIGHT*CLUSTER_SIZE)return NULL;
	return &wch->data[p.y/CLUSTER_SIZE];
}

s16 getWorldBlock(world_s* w, vect3Di_s p)
{
	if(!w)return -1;
	p=vaddi(p,vmuli(w->position,-CLUSTER_SIZE));
	if(p.x<0 || p.y<0 || p.z<0)return -1;
	if(p.x>=WORLD_SIZE*CLUSTER_SIZE || p.y>=CHUNK_HEIGHT*CLUSTER_SIZE || p.z>=WORLD_SIZE*CLUSTER_SIZE)return -1;

	return getWorldChunkBlock(w->data[p.x/CLUSTER_SIZE][p.z/CLUSTER_SIZE], vect3Di(p.x%CLUSTER_SIZE, p.y, p.z%CLUSTER_SIZE));
}

worldChunk_s* getWorldChunk(world_s* w, vect3Di_s p) //in chunk coord
{
	if(!w)return NULL;
	p=vaddi(p,vmuli(w->position,-1));
	if(p.x<0 || p.y<0 || p.z<0)return NULL;
	if(p.x>=WORLD_SIZE || p.y>=CHUNK_HEIGHT || p.z>=WORLD_SIZE)return NULL;

	return w->data[p.x][p.z];
}

void alterWorldClusterBlock(worldCluster_s* wcl, world_s* w, vect3Di_s p, u8 block, bool regenerate)
{
	if(!wcl || (wcl->status&WCL_DATA_UNAVAILABLE))return;
	if(p.x<0 || p.y<0 || p.z<0)return;
	if(p.x>=CLUSTER_SIZE || p.y>=CLUSTER_SIZE || p.z>=CLUSTER_SIZE)return;

	wcl->data[p.x][p.y][p.z]=block;
	if(regenerate)generateWorldClusterGeometry(wcl, w, NULL, 0);
}

void alterWorldChunkBlock(worldChunk_s* wch, world_s* w, vect3Di_s p, u8 block, bool regenerate)
{
	if(!wch)return;
	if(p.x<0 || p.y<0 || p.z<0)return;
	if(p.x>=CLUSTER_SIZE || p.y>=CHUNK_HEIGHT*CLUSTER_SIZE || p.z>=CLUSTER_SIZE)return;

	u16 clusterY=p.y/CLUSTER_SIZE;
	p.y%=CLUSTER_SIZE;
	alterWorldClusterBlock(&wch->data[clusterY], w, vect3Di(p.x, p.y, p.z), block, regenerate);

	wch->modified=true;

	if(regenerate)
	{
		if(!p.y && clusterY)generateWorldClusterGeometry(&wch->data[clusterY-1], w, NULL, 0);
		else if(p.y==CLUSTER_SIZE-1 && clusterY<CHUNK_HEIGHT-1)generateWorldClusterGeometry(&wch->data[clusterY+1], w, NULL, 0);
	}
}

void alterWorldBlock(world_s* w, vect3Di_s p, u8 block, bool regenerate)
{
	if(!w)return;
	p=vaddi(p,vmuli(w->position,-CLUSTER_SIZE));
	if(p.x<0 || p.y<0 || p.z<0)return;
	if(p.x>=WORLD_SIZE*CLUSTER_SIZE || p.y>=CHUNK_HEIGHT*CLUSTER_SIZE || p.z>=WORLD_SIZE*CLUSTER_SIZE)return;

	u16 clusterX=p.x/CLUSTER_SIZE, clusterZ=p.z/CLUSTER_SIZE;
	p.x%=CLUSTER_SIZE; p.z%=CLUSTER_SIZE;
	alterWorldChunkBlock(w->data[clusterX][clusterZ], w, vect3Di(p.x, p.y, p.z), block, regenerate);

	if(regenerate)
	{
		u16 clusterY=p.y/CLUSTER_SIZE;
		if(!p.x && clusterX && w->data[clusterX-1][clusterZ])generateWorldClusterGeometry(&w->data[clusterX-1][clusterZ]->data[clusterY], w, NULL, 0);
		else if(p.x==CLUSTER_SIZE-1 && clusterX<WORLD_SIZE-1 && w->data[clusterX+1][clusterZ])generateWorldClusterGeometry(&w->data[clusterX+1][clusterZ]->data[clusterY], w, NULL, 0);
		if(!p.z && clusterZ && w->data[clusterX][clusterZ-1])generateWorldClusterGeometry(&w->data[clusterX][clusterZ-1]->data[clusterY], w, NULL, 0);
		else if(p.z==CLUSTER_SIZE-1 && clusterZ<WORLD_SIZE-1 && w->data[clusterX][clusterZ+1])generateWorldClusterGeometry(&w->data[clusterX][clusterZ+1]->data[clusterY], w, NULL, 0);
	}
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
					initWorldChunk(w->data[i][j], w, vect3Di(i+w->position.x,0,j+w->position.z));
					dispatchJob(NULL, createJobGenerateChunkData(w->data[i][j]));
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
			if(i-v.x >= 0 && i-v.x < WORLD_SIZE && j-v.z >= 0 && j-v.z < WORLD_SIZE)w->data[i-v.x][j-v.z]=tmpData[i][j];
			else{freeChunk(tmpData[i][j]); tmpData[i][j]=NULL;}
		}
	}

	w->position=vaddi(w->position,v);
}

void drawWorld(world_s* w, camera_s* c)
{
	if(!w)return;

	int i, j;
	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			drawWorldChunk(w, w->data[i][j], c);
		}
	}
}
