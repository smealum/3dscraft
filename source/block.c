#include <3ds/3ds.h>
#include "world.h"
#include "block.h"

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

s16 blockShouldBeFace(u8 currentBlock, u8 nextBlock)
{
	if(currentBlock && nextBlock)return -1;
	if(!currentBlock && !nextBlock)return -1;
	if(currentBlock)return currentBlock;
	return nextBlock;
}

void blockGenerateFaceGeometry(blockFace_s* bf, gsVbo_s* vbo)
{
	if(!bf || !vbo)return;
	if(vbo->maxSize-vbo->currentSize < FACE_VBO_SIZE)return;

	const vect3Df_s p=vi2f(bf->position);
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

	//triangle 1
		v.position=vertices[0];
		v.texcoord[0]=0.0f; v.texcoord[1]=0.0f;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s));

		v.position=vertices[1];
		v.texcoord[0]=1.0f; v.texcoord[1]=0.0f;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s));

		v.position=vertices[2];
		v.texcoord[0]=1.0f; v.texcoord[1]=1.0f;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s));

	//triangle 2
		v.position=vertices[0];
		v.texcoord[0]=0.0f; v.texcoord[1]=0.0f;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s));

		v.position=vertices[2];
		v.texcoord[0]=1.0f; v.texcoord[1]=1.0f;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s));

		v.position=vertices[3];
		v.texcoord[0]=0.0f; v.texcoord[1]=1.0f;
		gsVboAddData(vbo, &v, sizeof(faceVertex_s));
}
