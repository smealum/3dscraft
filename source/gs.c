#include <stdlib.h>
#include <string.h>
#include <3ds/3ds.h>

#include "gs.h"
#include "math.h"

static void gsInitMatrixStack();

//----------------------
//   GS SYSTEM STUFF
//----------------------

void gsInit(void)
{
	gsInitMatrixStack();
}

void gsExit(void)
{

}

//----------------------
//  MATRIX STACK STUFF
//----------------------

static mtx44 gsMatrixStacks[GS_MATRIXTYPES][GS_MATRIXSTACK_SIZE];
static u32 gsMatrixStackRegisters[GS_MATRIXTYPES]={0x20, 0x24};
static u8 gsMatrixStackOffsets[GS_MATRIXTYPES];
static bool gsMatrixStackUpdated[GS_MATRIXTYPES];
static GS_MATRIX gsCurrentMatrixType;

static void gsInitMatrixStack()
{
	int i;
	for(i=0; i<GS_MATRIXTYPES; i++)
	{
		gsMatrixStackOffsets[i]=0;
		gsMatrixStackUpdated[i]=true;
		loadIdentity44((float*)gsMatrixStacks[i][0]);
	}
	gsCurrentMatrixType=GS_PROJECTION;
}

float* gsGetMatrix(GS_MATRIX m)
{
	if(m<0 || m>=GS_MATRIXTYPES)return NULL;
	
	return (float*)gsMatrixStacks[m][gsMatrixStackOffsets[m]];
}

int gsLoadMatrix(GS_MATRIX m, float* data)
{
	if(m<0 || m>=GS_MATRIXTYPES || !data)return -1;
	
	memcpy(gsGetMatrix(m), data, sizeof(mtx44));

	gsMatrixStackUpdated[m]=true;

	return 0;
}

int gsPushMatrix()
{
	const GS_MATRIX m=gsCurrentMatrixType;
	if(m<0 || m>=GS_MATRIXTYPES)return -1;
	if(gsMatrixStackOffsets[m]<0 || gsMatrixStackOffsets[m]>=GS_MATRIXSTACK_SIZE-1)return -1;

	float* cur=gsGetMatrix(m);
	gsMatrixStackOffsets[m]++;
	memcpy(gsGetMatrix(m), cur, sizeof(mtx44));

	return 0;
}

int gsPopMatrix()
{
	const GS_MATRIX m=gsCurrentMatrixType;
	if(m<0 || m>=GS_MATRIXTYPES)return -1;
	if(gsMatrixStackOffsets[m]<1 || gsMatrixStackOffsets[m]>=GS_MATRIXSTACK_SIZE)return -1;

	gsMatrixStackOffsets[m]--;

	gsMatrixStackUpdated[m]=true;

	return 0;
}

int gsMatrixMode(GS_MATRIX m)
{
	if(m<0 || m>=GS_MATRIXTYPES)return -1;

	gsCurrentMatrixType=m;

	return 0;
}

//------------------------
// MATRIX TRANSFORM STUFF
//------------------------

int gsMultMatrix(float* data)
{
	if(!data)return -1;
	
	mtx44 tmp;
	multMatrix44(gsGetMatrix(gsCurrentMatrixType), data, (float*)tmp);
	memcpy(gsGetMatrix(gsCurrentMatrixType), (float*)tmp, sizeof(mtx44));

	gsMatrixStackUpdated[gsCurrentMatrixType]=true;

	return 0;
}

void gsLoadIdentity()
{
	loadIdentity44(gsGetMatrix(gsCurrentMatrixType));
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsProjectionMatrix(float fovy, float aspect, float near, float far)
{
	initProjectionMatrix(gsGetMatrix(gsCurrentMatrixType), fovy, aspect, near, far);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsRotateX(float x)
{
	rotateMatrixX(gsGetMatrix(gsCurrentMatrixType), x);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsRotateY(float y)
{
	rotateMatrixY(gsGetMatrix(gsCurrentMatrixType), y);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsRotateZ(float z)
{
	rotateMatrixZ(gsGetMatrix(gsCurrentMatrixType), z);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsScale(float x, float y, float z)
{
	scaleMatrix(gsGetMatrix(gsCurrentMatrixType), x, y, z);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

void gsTranslate(float x, float y, float z)
{
	translateMatrix(gsGetMatrix(gsCurrentMatrixType), x, y, z);
	gsMatrixStackUpdated[gsCurrentMatrixType]=true;
}

//----------------------
// MATRIX RENDER STUFF
//----------------------

static void gsSetUniformMatrix(u32 startreg, float* m)
{
	float param[16];

	param[0x0]=m[3]; //w
	param[0x1]=m[2]; //z
	param[0x2]=m[1]; //y
	param[0x3]=m[0]; //x

	param[0x4]=m[7];
	param[0x5]=m[6];
	param[0x6]=m[5];
	param[0x7]=m[4];
	
	param[0x8]=m[11];
	param[0x9]=m[10];
	param[0xa]=m[9];
	param[0xb]=m[8];

	param[0xc]=m[15];
	param[0xd]=m[14];
	param[0xe]=m[13];
	param[0xf]=m[12];

	GPU_SetUniform(startreg, (u32*)param, 4);
}

static int gsUpdateTransformation()
{
	GS_MATRIX m;
	for(m=0; m<GS_MATRIXTYPES; m++)
	{
		if(gsMatrixStackUpdated[m])
		{
			gsSetUniformMatrix(gsMatrixStackRegisters[m], gsGetMatrix(m));
			gsMatrixStackUpdated[m]=false;
		}
	}
	return 0;
}

//----------------------
//      VBO STUFF
//----------------------

int gsVboInit(gsVbo_s* vbo)
{
	if(!vbo)return -1;

	vbo->data=NULL;
	vbo->currentSize=0;
	vbo->maxSize=0;

	return 0;
}

int gsVboCreate(gsVbo_s* vbo, u32 size)
{
	if(!vbo)return -1;

	//TODO : implement some real allocation
	vbo->data=gfxAllocLinear(size);
	vbo->currentSize=0;
	vbo->maxSize=size;

	return 0;
}

void* gsVboGetOffset(gsVbo_s* vbo)
{
	if(!vbo)return NULL;

	return (void*)(&((u8*)vbo->data)[vbo->currentSize]);
}

int gsVboAddData(gsVbo_s* vbo, void* data, u32 size)
{
	if(!vbo || !data || !size)return -1;
	if(vbo->maxSize-vbo->currentSize < size)return -1;

	memcpy(gsVboGetOffset(vbo), data, size);
	vbo->currentSize+=size;

	return 0;
}

int gsVboDestroy(gsVbo_s* vbo)
{
	if(!vbo)return -1;

	//nothing to do until we have a real allocator
	//so for now just reset it
	gsVboInit(vbo);

	return 0;
}

int gsVboDraw(gsVbo_s* vbo)
{
	if(!vbo)return -1;

	gsUpdateTransformation();

	GPU_SetAttributeBuffers(3, (u32*)osConvertVirtToPhys((u32)vbo->data),
		GPU_ATTRIBFMT(0, 3, GPU_FLOAT)|GPU_ATTRIBFMT(1, 2, GPU_FLOAT)|GPU_ATTRIBFMT(2, 3, GPU_FLOAT),
		0xFFC, 0x210, 1, (u32[]){0x00000000}, (u64[]){0x210}, (u8[]){3});
	GPU_DrawArray(GPU_TRIANGLES, vbo->currentSize);

	return 0;
}
