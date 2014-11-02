#include <3ds.h>
#include "world.h"
#include "block.h"

const blockType_s blockTypes[BLOCK_NUM] =
	{
		(blockType_s){5,3,4}, //BLOCK_AIR
		(blockType_s){2,2,2}, //BLOCK_DIRT
		(blockType_s){0,2,3} //BLOCK_GRASS
	};

const vect3Df_s fo[]={(vect3Df_s){0.0f,0.0f,0.0f}, //MX
						(vect3Df_s){1.0f,0.0f,1.0f}, //PX
						(vect3Df_s){0.0f,0.0f,0.0f}, //MY
						(vect3Df_s){0.0f,1.0f,0.0f}, //PY
						(vect3Df_s){1.0f,0.0f,0.0f}, //MZ
						(vect3Df_s){0.0f,0.0f,1.0f}, //PZ
						};

const vect3Df_s fd1[]={(vect3Df_s){0.0f,0.0f,1.0f}, //MX
						(vect3Df_s){0.0f,0.0f,-1.0f}, //PX
						(vect3Df_s){1.0f,0.0f,0.0f}, //MY
						(vect3Df_s){0.0f,0.0f,1.0f}, //PY
						(vect3Df_s){-1.0f,0.0f,0.0f}, //MZ
						(vect3Df_s){1.0f,0.0f,0.0f}, //PZ
						};

const vect3Df_s fd2[]={(vect3Df_s){0.0f,1.0f,0.0f}, //MX
						(vect3Df_s){0.0f,1.0f,0.0f}, //PX
						(vect3Df_s){0.0f,0.0f,1.0f}, //MY
						(vect3Df_s){1.0f,0.0f,0.0f}, //PY
						(vect3Df_s){0.0f,1.0f,0.0f}, //MZ
						(vect3Df_s){0.0f,1.0f,0.0f}, //PZ
						};

const vect3Df_s fn[]={(vect3Df_s){-1.0f,0.0f,0.0f}, //MX
						(vect3Df_s){1.0f,0.0f,1.0f}, //PX
						(vect3Df_s){0.0f,-1.0f,0.0f}, //MY
						(vect3Df_s){0.0f,1.0f,0.0f}, //PY
						(vect3Df_s){0.0f,0.0f,-1.0f}, //MZ
						(vect3Df_s){0.0f,0.0f,1.0f}, //PZ
						};

s16 blockShouldBeFace(s16 currentBlock, s16 nextBlock)
{
	if(currentBlock<0 || nextBlock<0)return -1;
	if(currentBlock>=BLOCK_NUM || nextBlock>=BLOCK_NUM)return -1;
	if(currentBlock!=BLOCK_AIR && nextBlock!=BLOCK_AIR)return -1;
	if(currentBlock==BLOCK_AIR && nextBlock==BLOCK_AIR)return -1;
	if(currentBlock!=BLOCK_AIR)return currentBlock;
	return -1;
}

void blockGenerateFaceGeometry(blockFace_s* bf, gsVbo_s* vbo, vect3Df_s off)
{
	if(!bf || !vbo)return;
	if(vbo->maxSize-vbo->currentSize < FACE_VBO_SIZE)return;

	const vect3Df_s p=vaddf(vi2f(bf->position), off);
	const orientation_t o=bf->orientation;
	const vect3Df_s vertices[]=
	{
		vaddf(p, fo[o]),
		vaddf(p, vaddf(fo[o], fd1[o])),
		vaddf(p, vaddf(vaddf(fo[o], fd1[o]), fd2[o])),
		vaddf(p, vaddf(fo[o], fd2[o]))
	};

	faceVertex_s v;
	v.normal=fn[o];

	vect3Df_s t=blockFaceGetTexcoord(bf);

	//triangle 1
		v.position=vertices[0];
		v.texcoord[0]=t.x+0.0f; v.texcoord[1]=t.y+BLOCK_TEXTURE_SIZE;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s), 1);

		v.position=vertices[1];
		v.texcoord[0]=t.x+BLOCK_TEXTURE_SIZE; v.texcoord[1]=t.y+BLOCK_TEXTURE_SIZE;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s), 1);

		v.position=vertices[2];
		v.texcoord[0]=t.x+BLOCK_TEXTURE_SIZE; v.texcoord[1]=t.y+0.0f;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s), 1);

	//triangle 2
		v.position=vertices[0];
		v.texcoord[0]=t.x+0.0f; v.texcoord[1]=t.y+BLOCK_TEXTURE_SIZE;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s), 1);

		v.position=vertices[2];
		v.texcoord[0]=t.x+BLOCK_TEXTURE_SIZE; v.texcoord[1]=t.y+0.0f;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s), 1);

		v.position=vertices[3];
		v.texcoord[0]=t.x+0.0f; v.texcoord[1]=t.y+0.0f;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s), 1);
}

void faceGetTexcoord(u16 n, float* u, float* v)
{
	if(u)*u=((float)(n&0xF))*BLOCK_TEXTURE_SIZE;
	if(v)*v=((float)(n>>4))*BLOCK_TEXTURE_SIZE;
}

vect3Df_s blockFaceGetTexcoord(blockFace_s* bf)
{
	if(!bf || bf->type>=BLOCK_NUM)return vect3Df(0,0,0);

	vect3Df_s ret;
	u16 n=blockTypes[bf->type].side;
	if(bf->orientation==FACE_PY)n=blockTypes[bf->type].top;
	else if(bf->orientation==FACE_MY)n=blockTypes[bf->type].bottom;
	faceGetTexcoord(n, &ret.x, &ret.y);
	return ret;
}
