#ifndef CURSOR_H
#define CURSOR_H

#include <3ds.h>

#include "math.h"
#include "gs.h"

typedef struct
{
	vect3Di_s position;
	u8 direction;
	gsVbo_s vbo;
	bool active;
}cursor_s;

void initCursor(cursor_s* c);
void drawCursor(cursor_s* c);

#endif
