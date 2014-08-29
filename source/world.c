#include <string.h>
#include <3ds.h>

#include "gs.h"
#include "world.h"

#define pushFace(l, s, f) ((l)[(s)++]=f)
#define popFace(l, s) ((s)?(&((l)[--(s)])):NULL)

void initWorldCluster(worldCluster_s* wcl, vect3Di_s pos)
{
	if(!wcl)return;

	memset(wcl->data, 0x00, CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE);
	wcl->position=pos;
	gsVboInit(&wcl->vbo);
	wcl->generated=false;
}

void drawWorldCluster(worldCluster_s* wcl)
{
	if(!wcl || !wcl->generated)return;

	//TODO : transformations
	gsVboDraw(&wcl->vbo);
}

void generateWorldClusterGeometry(worldCluster_s* wcl)
{
	if(!wcl)return;
	if(wcl->generated)gsVboDestroy(&wcl->vbo);

	wcl->generated=false;

	//first, we go through the whole cluster to generate a "list" of faces
	static blockFace_s faceList[4096]; //TODO : calculate real max
	static int faceListSize=0;

	const vect3Di_s p = vmuli(wcl->position, CLUSTER_SIZE);
	int i, j, k;
	for(i=1; i<CLUSTER_SIZE-1; i++)
	{
		for(j=1; j<CLUSTER_SIZE-1; j++)
		{
			for(k=1; k<CLUSTER_SIZE-1; k++)
			{
				u8 cb=wcl->data[i][j][k];
				if(blockShouldBeFace(cb, wcl->data[i+1][j][k])>=0)pushFace(faceList, faceListSize, blockFace(FACE_PX, vaddi(p, vect3Di(i,j,k))));
				if(blockShouldBeFace(cb, wcl->data[i-1][j][k])>=0)pushFace(faceList, faceListSize, blockFace(FACE_MX, vaddi(p, vect3Di(i,j,k))));
				if(blockShouldBeFace(cb, wcl->data[i][j+1][k])>=0)pushFace(faceList, faceListSize, blockFace(FACE_PY, vaddi(p, vect3Di(i,j,k))));
				if(blockShouldBeFace(cb, wcl->data[i][j-1][k])>=0)pushFace(faceList, faceListSize, blockFace(FACE_MY, vaddi(p, vect3Di(i,j,k))));
				if(blockShouldBeFace(cb, wcl->data[i][j][k+1])>=0)pushFace(faceList, faceListSize, blockFace(FACE_PZ, vaddi(p, vect3Di(i,j,k))));
				if(blockShouldBeFace(cb, wcl->data[i][j][k-1])>=0)pushFace(faceList, faceListSize, blockFace(FACE_MZ, vaddi(p, vect3Di(i,j,k))));
			}
		}
	}

	//then, we set up VBO size to create the VBO
	const u32 size=faceListSize*FACE_VBO_SIZE;

	if(!gsVboCreate(&wcl->vbo, size))
	{
		//and if that succeeds, we transfer all those faces to the VBO !
		blockFace_s* bf;
		while((bf=popFace(faceList, faceListSize)))
		{
			blockGenerateFaceGeometry(bf, &wcl->vbo);
		}

		wcl->generated=true;
	}
}

void generateWorldClusterData(worldCluster_s* wcl)
{
	if(!wcl)return;
	if(wcl->generated)gsVboDestroy(&wcl->vbo);

	//TEMP
	if(wcl->position.y<8)memset(wcl->data, BLOCK_DIRT, CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE);
	else if(wcl->position.y==8)
	{
		memset(wcl->data, BLOCK_AIR, CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE);
		memset(wcl->data, BLOCK_DIRT, CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE/2);
	}else memset(wcl->data, BLOCK_AIR, CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE);
}

void initWorldChunk(worldChunk_s* wch, vect3Di_s pos)
{
	if(!wch)return;

	int k; for(k=0; k<CHUNK_HEIGHT; k++)initWorldCluster(&wch->data[k], vect3Di(pos.x, k, pos.z));
	wch->position=pos;
}

//TEMP ?
void generateWorldChunkData(worldChunk_s* wch)
{
	if(!wch)return;

	int k; for(k=0; k<CHUNK_HEIGHT; k++)generateWorldClusterData(&wch->data[k]);
}

//TEMP ?
void generateWorldChunkGeometry(worldChunk_s* wch)
{
	if(!wch)return;

	int k; for(k=0; k<CHUNK_HEIGHT; k++)generateWorldClusterGeometry(&wch->data[k]);
}

void drawWorldChunk(worldChunk_s* wch)
{
	if(!wch)return;

	//culling goes here
	int k; for(k=0; k<CHUNK_HEIGHT; k++)drawWorldCluster(&wch->data[k]);
}

void initWorld(world_s* w)
{
	if(!w)return;

	int i, j;
	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			initWorldChunk(&w->data[i][j], vect3Di(i,0,j));
		}
	}
}

//TEMP
void generateWorld(world_s* w)
{
	if(!w)return;
	int i, j;

	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			generateWorldChunkData(&w->data[i][j]);
		}
	}

	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			generateWorldChunkGeometry(&w->data[i][j]);
		}
	}
}

void drawWorld(world_s* w)
{
	if(!w)return;

	int i, j;
	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			//culling goes here
			drawWorldChunk(&w->data[i][j]);
		}
	}
}
