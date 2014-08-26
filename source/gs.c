#include <stdlib.h>
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
	vbo->size=0;

	return 0;
}

int gsVboCreate(gsVbo_s* vbo, u32 size)
{
	if(!vbo)return -1;

	//TODO : implement some real allocation
	vbo->data=gfxAllocLinear(size);
	vbo->size=size;

	return 0;
}

int gsVboDestroy(gsVbo_s* vbo)
{
	if(!vbo)return -1;

	//nothing to do until we have a real allocator
	vbo->data=NULL;
	vbo->size=0;

	return 0;
}

int gsVboDraw(gsVbo_s* vbo)
{
	if(!vbo)return -1;

	//TODO

	return 0;
}
