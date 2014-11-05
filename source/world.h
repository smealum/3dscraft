#ifndef WORLD_H
#define WORLD_H

#include <3ds.h>
#include "math.h"
#include "block.h"
#include "camera.h"

#define CLUSTER_SIZE (8)

#define CHUNK_HEIGHT (16)

#define WORLD_SIZE (16)
// #define WORLD_SIZE (16)
// #define WORLD_SIZE (8)

typedef enum
{
	WCL_DATA_UNAVAILABLE = BIT(1),
	WCL_GEOM_UNAVAILABLE = BIT(2),
	WCL_BUSY = BIT(3),
}worldClusterStatus_t;

typedef struct
{
	u8 data[CLUSTER_SIZE][CLUSTER_SIZE][CLUSTER_SIZE];
	vect3Di_s position; //in cluster coordinates
	gsVbo_s vbo;
	worldClusterStatus_t status;
}worldCluster_s;

typedef struct
{
	u8 elevation;
}worldColumnInfo_s;

typedef struct worldChunk_s
{
	worldCluster_s data[CHUNK_HEIGHT];
	worldColumnInfo_s info[CLUSTER_SIZE][CLUSTER_SIZE];
	vect3Di_s position; //in cluster coordinates (actually 2D)
	struct worldChunk_s* next; //for chunk pool and chunk cache
}worldChunk_s;

typedef struct
{
	worldChunk_s* data[WORLD_SIZE][WORLD_SIZE];
	vect3Di_s position; //in cluster coordinates (actually 2D)
}world_s;

void generateWorldChunkData(worldChunk_s* wch);
void generateWorldClusterGeometry(worldCluster_s* wcl, world_s* w, blockFace_s* tmpBuffer, int tmpBufferSize);

void initChunkPool(void);
worldChunk_s* getNewChunk(void);
void freeChunk(worldChunk_s* wch);
void fixChunk(worldChunk_s* wch);
bool isChunkBusy(worldChunk_s* wch);

void initWorld(world_s* w);
void translateWorld(world_s* w, vect3Di_s v);
void generateWorld(world_s* w);
void updateWorld(world_s* w);
void drawWorld(world_s* w, camera_s* c);

s16 getWorldBlock(world_s* w, vect3Di_s p);
worldCluster_s* getWorldBlockCluster(world_s* w, vect3Di_s p);
void alterWorldBlock(world_s* w, vect3Di_s p, u8 block, bool regenerate);

#endif
