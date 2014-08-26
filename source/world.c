#include <string.h>
#include <3ds/3ds.h>

#include "gs.h"
#include "world.h"

#define pushFace(f, l, s) ((l)[(s)++]=f)

void initWorldCluster(worldCluster_s* wcl)
{
	if(!wcl)return;

	memset(wcl->data, 0x00, CLUSTER_SIZE*CLUSTER_SIZE*CLUSTER_SIZE);
	gsVboInit(&wcl->vbo);
	wcl->generated=false;
}

void drawWorldCluster(worldCluster_s* wcl)
{
	if(!wcl || !wcl->generated)return;

	//TODO : transformations
	gsVboDraw(&wcl->vbo);
}

void generateWorldCluster(worldCluster_s* wcl, vect3Di_s pos)
{
	if(!wcl)return;
	if(wcl->generated)gsVboDestroy(&wcl->vbo);

	wcl->generated=false;

	//first, we go through the whole cluster to generate a "list" of faces
	static blockFace_s faceList[4096]; //TODO : calculate real max
	static faceListSize=0;
	int i, j, k;
	for(i=1; i<CLUSTER_SIZE-1; i++)
	{
		for(j=1; j<CLUSTER_SIZE-1; j++)
		{
			for(k=1; k<CLUSTER_SIZE-1; k++)
			{
				u8 cb=wcl->data[i][j][k];
				if(blockShouldBeFace(cb, wcl->data[i+1][j][k])>=0)pushFace(blockFace(FACE_PX, vect3Di(i,j,k)), faceList, faceListSize);
				if(blockShouldBeFace(cb, wcl->data[i-1][j][k])>=0)pushFace(blockFace(FACE_MX, vect3Di(i,j,k)), faceList, faceListSize);
				if(blockShouldBeFace(cb, wcl->data[i][j+1][k])>=0)pushFace(blockFace(FACE_PY, vect3Di(i,j,k)), faceList, faceListSize);
				if(blockShouldBeFace(cb, wcl->data[i][j-1][k])>=0)pushFace(blockFace(FACE_MY, vect3Di(i,j,k)), faceList, faceListSize);
				if(blockShouldBeFace(cb, wcl->data[i][j][k+1])>=0)pushFace(blockFace(FACE_PZ, vect3Di(i,j,k)), faceList, faceListSize);
				if(blockShouldBeFace(cb, wcl->data[i][j][k-1])>=0)pushFace(blockFace(FACE_MZ, vect3Di(i,j,k)), faceList, faceListSize);
			}
		}
	}

	//then, we set up VBO size to create the VBO
	u32 size=0;

	if(!gsVboCreate(&wcl->vbo, size))
	{
		//and if that succeeds, we transfer all those faces to the VBO !

		wcl->generated=true;
	}
}

void initWorldChunk(worldChunk_s* wch)
{
	if(!wch)return;

	int i;
	for(i=0; i<CHUNK_HEIGHT; i++)initWorldCluster(&wch->data[i]);
}

void drawWorldChunk(worldChunk_s* wch)
{
	if(!wch)return;

	int i;
	//culling goes here
	for(i=0; i<CHUNK_HEIGHT; i++)drawWorldCluster(&wch->data[i]);
}

void initWorld(world_s* w)
{
	if(!w)return;

	int i, j;
	for(i=0; i<WORLD_SIZE; i++)
	{
		for(j=0; j<WORLD_SIZE; j++)
		{
			initWorldChunk(&w->data[i][j]);
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

blockFace_s blockFace(orientation_t o, vect3Di_s p)
{
	return (blockFace_s){o,p};
}

s16 blockShouldBeFace(u8 currentBlock, u8 nextBlock)
{
	if(currentBlock && nextBlock)return -1;
	if(!currentBlock && !nextBlock)return -1;
	if(currentBlock)return currentBlock;
	return nextBlock;
}
