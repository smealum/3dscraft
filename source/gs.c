#include <stdlib.h>
#include <string.h>
#include <3ds/3ds.h>

#include "gs.h"

void gsInit(void)
{

}

void gsExit(void)
{

}

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

	GPU_SetAttributeBuffers(3, (u32*)osConvertVirtToPhys((u32)vbo->data),
		GPU_ATTRIBFMT(0, 3, GPU_FLOAT)|GPU_ATTRIBFMT(1, 2, GPU_FLOAT)|GPU_ATTRIBFMT(2, 3, GPU_FLOAT),
		0xFFC, 0x210, 1, (u32[]){0x00000000}, (u64[]){0x210}, (u8[]){3});
	GPU_DrawArray(GPU_TRIANGLES, vbo->currentSize);

	return 0;
}
