#include <3ds.h>

#include "cursor.h"
#include "block.h"

#define CURSOR_SCALEFACT (1.1f)

const faceVertex_s cursorVboData[]=
{
	//first triangle
	{(vect3Df_s){-0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, -0.5f*CURSOR_SCALEFACT}, (float[]){0.0f, 0.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
	{(vect3Df_s){-0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, +0.5f*CURSOR_SCALEFACT}, (float[]){0.0f, 0.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
	{(vect3Df_s){+0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, +0.5f*CURSOR_SCALEFACT}, (float[]){0.0f, 0.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
	//first triangle
	{(vect3Df_s){+0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, +0.5f*CURSOR_SCALEFACT}, (float[]){0.0f, 0.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
	{(vect3Df_s){+0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, -0.5f*CURSOR_SCALEFACT}, (float[]){0.0f, 0.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
	{(vect3Df_s){-0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, -0.5f*CURSOR_SCALEFACT}, (float[]){0.0f, 0.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
};

void initCursor(cursor_s* c)
{
	if(!c)return;

	c->position=vect3Di(0,0,0);
	c->direction=0;
	c->active=false;

	gsVboInit(&c->vbo);
	gsVboCreate(&c->vbo, sizeof(cursorVboData));
	gsVboAddData(&c->vbo, (void*)cursorVboData, sizeof(cursorVboData), sizeof(cursorVboData)/sizeof(faceVertex_s));
	gsVboFlushData(&c->vbo);
}

void drawCursor(cursor_s* c)
{
	if(!c || !c->active)return;

	gsPushMatrix();
		gsTranslate(c->position.x, c->position.y, c->position.z);
		gsTranslate(0.5f, 0.5f, 0.5f);
		switch(c->direction)
		{
			case 0:
				gsRotateZ(M_PI/2);
				break;
			case 1:
				gsRotateZ(-M_PI/2);
				break;
			case 2:
				gsRotateY(M_PI);
				break;
			case 4:
				gsRotateX(-M_PI/2);
				break;
			case 5:
				gsRotateX(M_PI/2);
				break;
		}
		gsVboDraw(&c->vbo);
	gsPopMatrix();
}
