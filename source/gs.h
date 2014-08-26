#ifndef GS_H
#define GS_H

#include <3ds/types.h>

typedef struct
{
	void* data;
	u32 size;
}gsVbo_s;

void gsInit(void);
void gsExit(void);

int gsVboInit(gsVbo_s* vbo);
int gsVboCreate(gsVbo_s* vbo, u32 size);
int gsVboDestroy(gsVbo_s* vbo);
int gsVboDraw(gsVbo_s* vbo);

#endif
