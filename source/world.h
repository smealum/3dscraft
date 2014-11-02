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

typedef struct worldChunk_s
{
	worldCluster_s data[CHUNK_HEIGHT];
	vect3Di_s position; //in cluster coordinates (actually 2D)
	struct worldChunk_s* next; //for chunk pool and chunk cache
}worldChunk_s;

typedef struct
{
	worldChunk_s* data[WORLD_SIZE][WORLD_SIZE];
	vect3Di_s position; //in cluster coordinates (actually 2D)
}world_s;

void initChunkPool(void);
worldChunk_s* getNewChunk(void);
void freeChunk(worldChunk_s* wch);

void initWorld(world_s* w);
void translateWorld(world_s* w, vect3Di_s v);
void generateWorld(world_s* w);
void updateWorld(world_s* w);
void drawWorld(world_s* w);

s16 getWorldBlock(world_s* w, vect3Di_s p);

#endif
