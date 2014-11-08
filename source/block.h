#ifndef BLOCK_H
#define BLOCK_H

#include <3ds/types.h>
#include "math.h"
#include "gs.h"

#define VERTEX_VBO_SIZE (sizeof(faceVertex_s))
#define FACE_VBO_SIZE (VERTEX_VBO_SIZE*6)

#define BLOCK_TEXTURE_SIZE (1.0f/16)

typedef enum
{
	BLOCK_AIR = 0,
	BLOCK_DIRT,
	BLOCK_GRASS,
	BLOCK_STONE,
	BLOCK_COBBLESTONE,
	BLOCK_PLANK,
	BLOCK_BRICK,
	BLOCK_BEDROCK,
	BLOCK_SAND,
	BLOCK_GRAVEL,
	BLOCK_WOOD,
	BLOCK_IRON,
	BLOCK_GOLD,
	BLOCK_DIAMOND,
	BLOCK_EMERALD,
	BLOCK_IRONORE,
	BLOCK_GOLDORE,
	BLOCK_DIAMONDORE,
	BLOCK_COALORE,
	BLOCK_MOSSSTONE,
	BLOCK_OBSIDIAN,
	BLOCK_GLASS,
	BLOCK_NUM
}block_t;

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
	block_t type;
}blockFace_s;

typedef struct
{
	vect3Df_s position;
	float texcoord[2];
	vect3Df_s normal;
}faceVertex_s;

typedef struct
{
	u8 top, bottom, side;
}blockType_s;

const extern blockType_s blockTypes[BLOCK_NUM];

void blockGenerateFaceGeometry(blockFace_s* bf, gsVbo_s* vbo, vect3Df_s off);

s16 blockShouldBeFace(s16 currentBlock, s16 nextBlock);

vect3Df_s blockFaceGetTexcoord(blockFace_s* bf);

static inline blockFace_s blockFace(block_t t, orientation_t o, vect3Di_s p)
{
	return (blockFace_s){o,p,t};
}

#endif
