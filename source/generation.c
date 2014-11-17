#include <3ds.h>

#include "world.h"
#include "generation.h"
#include "sdnoise.h"

void initGeneration()
{

}

void exitGeneration()
{

}

int getWorldElevation(vect3Di_s p)
{
	return (int)(sdnoise2(((float)p.x)/(CLUSTER_SIZE*4), ((float)p.z)/(CLUSTER_SIZE*4), NULL, NULL)*CLUSTER_SIZE)+(CHUNK_HEIGHT*CLUSTER_SIZE/2);
}

void generateWorldChunkInfo(worldChunk_s* wch, worldChunkInfo_s* wci)
{
	if(!wch || !wci)return;

	int i, k;
	for(i=0; i<CLUSTER_SIZE; i++)
	{
		for(k=0; k<CLUSTER_SIZE; k++)
		{
			const vect3Di_s p=vect3Di(i+wch->position.x*CLUSTER_SIZE, 0, k+wch->position.z*CLUSTER_SIZE);
			wci->elevation[i][k]=getWorldElevation(p);
		}
	}
}

void generateWorldCluster(worldChunkInfo_s* wci, worldCluster_s* wcl)
{
	if(!wci || !wcl)return;

	//TEMP
	int i, j, k;
	for(i=0; i<CLUSTER_SIZE; i++)
	{
		for(k=0; k<CLUSTER_SIZE; k++)
		{
			//TEMP
			const vect3Di_s p=vect3Di(i+wcl->position.x*CLUSTER_SIZE, wcl->position.y*CLUSTER_SIZE, k+wcl->position.z*CLUSTER_SIZE);
			const int height=wci->elevation[i][k];
			for(j=0; j<CLUSTER_SIZE; j++)
			{
				if(p.y+j == height)wcl->data[i][j][k]=BLOCK_GRASS;
				else if(p.y+j < height)wcl->data[i][j][k]=BLOCK_DIRT;
				else wcl->data[i][j][k]=BLOCK_AIR;
			}
		}
	}
}
