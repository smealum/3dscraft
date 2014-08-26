#ifndef WORLD_H

#include <3ds/types.h>
#include "math.h"

#define CLUSTER_SIZE (8)

#define CHUNK_HEIGHT (16)

#define WORLD_SIZE (16)

typedef struct
{
	u8 data[CLUSTER_SIZE][CLUSTER_SIZE][CLUSTER_SIZE];
	gsVbo_s vbo;
	bool generated;
}worldCluster_s;

typedef struct
{
	worldCluster_s data[CHUNK_HEIGHT];
}worldChunk_s;

typedef struct
{
	worldChunk_s data[WORLD_SIZE][WORLD_SIZE];
}world_s;

typedef enum{
	FACE_MX,
	FACE_PX,
	FACE_MY,
	FACE_PY,
	FACE_MZ,
	FACE_PZ
}orientation_t;

typedef struct
{
	orientation_t orientation;
	vect3Di_s position;
}blockFace_s;

blockFace_s blockFace(orientation_t o, vect3Di_s p);
s16 blockShouldBeFace(u8 currentBlock, u8 nextBlock);

#endif
