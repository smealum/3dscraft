#ifndef WORLD_H
#define WORLD_H

#include <3ds.h>
#include "math.h"
#include "block.h"
#include "camera.h"
#include "streaming.h"

// #define CLUSTER_SIZE (8)
#define CLUSTER_SIZE (12)

#define CHUNK_HEIGHT (10)

// #define WORLD_SIZE (16)
#define WORLD_SIZE (12)

typedef enum
{
	WCL_PX = BIT(0),
	WCL_MX = BIT(1),
	WCL_PY = BIT(2),
	WCL_MY = BIT(3),
	WCL_PZ = BIT(4),
	WCL_MZ = BIT(5),
}worldClusterDirection_t;

typedef enum
{
	WCL_DATA_UNAVAILABLE = BIT(0),
	WCL_GEOM_UNAVAILABLE = BIT(1),
	WCL_BUSY = BIT(2),
}worldClusterStatus_t;

typedef struct
{
	u8 data[CLUSTER_SIZE][CLUSTER_SIZE][CLUSTER_SIZE];
	vect3Di_s position; //in cluster coordinates
	gsVbo_s vbo;
	u8 directions; //directions that have been taken into account when generating the VBO
	worldClusterStatus_t status;
}worldCluster_s;

typedef struct
{
	u8 elevation[CLUSTER_SIZE][CLUSTER_SIZE];
}worldChunkInfo_s;

struct world_s;

typedef struct worldChunk_s
{
	worldCluster_s data[CHUNK_HEIGHT];
	worldChunkInfo_s info;
	bool modified;
	vect3Di_s position; //in cluster coordinates (actually 2D)
	struct worldChunk_s* next; //for chunk pool and chunk cache
	struct world_s* world; //parent
}worldChunk_s;

typedef struct world_s
{
	worldStream_s stream;
	worldChunk_s* data[WORLD_SIZE][WORLD_SIZE];
	vect3Di_s position; //in cluster coordinates (actually 2D)
}world_s;

void generateWorldChunkData(worldChunk_s* wch);
void generateWorldClusterGeometry(worldCluster_s* wcl, world_s* w, blockFace_s* tmpBuffer, int tmpBufferSize);
int generateWorldAdditionalClusterGeometry(worldCluster_s* wcl, world_s* w, u8 directions, blockFace_s* tmpBuffer, int tmpBufferSize, gsVbo_s* vbo);
void generateWorldAdditionalGeometryList(worldCluster_s* wcl, world_s* w, u8 directions, blockFace_s* faceList, int faceBufferSize, int* faceListSize);
void buildClusterGeometry(worldCluster_s* wcl, blockFace_s* faceList, int faceBufferSize, int faceListSize, gsVbo_s* vbo);

void initChunkPool(void);
worldChunk_s* getNewChunk(void);
void freeChunk(worldChunk_s* wch);
void fixChunk(worldChunk_s* wch);
bool isChunkBusy(worldChunk_s* wch);

void initWorld(world_s* w);
void flushWorld(world_s* w);
void exitWorld(world_s* w);
void translateWorld(world_s* w, vect3Di_s v);
void generateWorld(world_s* w);
void updateWorld(world_s* w);
void drawWorld(world_s* w, camera_s* c);

s16 getWorldBlock(world_s* w, vect3Di_s p);
worldChunk_s* getWorldChunk(world_s* w, vect3Di_s p); //in chunk coord
worldCluster_s* getWorldBlockCluster(world_s* w, vect3Di_s p); //in block coord
void alterWorldBlock(world_s* w, vect3Di_s p, u8 block, bool regenerate);

#endif
