#ifndef WORLD_H
#define WORLD_H

#include <3ds/types.h>
#include "math.h"
#include "block.h"

#define CLUSTER_SIZE (8)

#define CHUNK_HEIGHT (16)

#define WORLD_SIZE (16)
// #define WORLD_SIZE (8)

typedef struct
{
	u8 data[CLUSTER_SIZE][CLUSTER_SIZE][CLUSTER_SIZE];
	vect3Di_s position; //in cluster coordinates
	gsVbo_s vbo;
	bool generated;
}worldCluster_s;

typedef struct
{
	worldCluster_s data[CHUNK_HEIGHT];
	vect3Di_s position; //in cluster coordinates (actually 2D)
}worldChunk_s;

typedef struct
{
	worldChunk_s data[WORLD_SIZE][WORLD_SIZE];
}world_s;


void initWorldCluster(worldCluster_s* wcl, vect3Di_s pos);
void drawWorldCluster(worldCluster_s* wcl);
void generateWorldClusterData(worldCluster_s* wcl);
void generateWorldClusterGeometry(worldCluster_s* wcl, world_s* w);

void initWorld(world_s* w);
void generateWorld(world_s* w);
void drawWorld(world_s* w);

s16 getWorldBlock(world_s* w, vect3Di_s p);

#endif
