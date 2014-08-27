#ifndef GS_H
#define GS_H

#include <3ds/types.h>

typedef struct
{
	u8* data;
	u32 currentSize; // in bytes
	u32 maxSize; // in bytes
}gsVbo_s;

void gsInit(void);
void gsExit(void);

int gsVboInit(gsVbo_s* vbo);
int gsVboCreate(gsVbo_s* vbo, u32 size);
int gsVboDestroy(gsVbo_s* vbo);
int gsVboDraw(gsVbo_s* vbo);
void* gsVboGetOffset(gsVbo_s* vbo);
int gsVboAddData(gsVbo_s* vbo, void* data, u32 size);

#endif
