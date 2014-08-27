#ifndef BLOCK_H
#define BLOCK_H

#include <3ds/types.h>
#include "math.h"
#include "gs.h"

#define VERTEX_VBO_SIZE (sizeof(faceVertex_s))
#define FACE_VBO_SIZE (VERTEX_VBO_SIZE*6)

typedef enum{
	FACE_MX = 0,
	FACE_PX = 1,
	FACE_MY = 2,
	FACE_PY = 3,
	FACE_MZ = 4,
	FACE_PZ = 5
}orientation_t;

typedef struct
{
	orientation_t orientation;
	vect3Di_s position;
}blockFace_s;

typedef struct
{
	vect3Df_s position;
	float texcoord[2];
	vect3Df_s normal;
}faceVertex_s;

void blockGenerateFaceGeometry(blockFace_s* bf, gsVbo_s* vbo);

s16 blockShouldBeFace(u8 currentBlock, u8 nextBlock);

static inline blockFace_s blockFace(orientation_t o, vect3Di_s p)
{
	return (blockFace_s){o,p};
}

#endif
