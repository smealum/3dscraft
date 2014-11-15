#include <3ds.h>

#include "cursor.h"
#include "block.h"

#include "cursor_bin.h"

#define CURSOR_SCALEFACT (1.1f)

u32* cursorTexture;

const faceVertex_s cursorVboData[]=
{
	//first triangle
	{(vect3Df_s){-0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, -0.5f*CURSOR_SCALEFACT}, (float[]){0.125f, 1.0f-0.125f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
	{(vect3Df_s){-0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, +0.5f*CURSOR_SCALEFACT}, (float[]){0.1875f, 1.0f-0.125f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
	{(vect3Df_s){+0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, +0.5f*CURSOR_SCALEFACT}, (float[]){0.1875f, 1.0f-0.0625f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
	//first triangle
	{(vect3Df_s){+0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, +0.5f*CURSOR_SCALEFACT}, (float[]){0.1875f, 1.0f-0.0625f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
	{(vect3Df_s){+0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, -0.5f*CURSOR_SCALEFACT}, (float[]){0.125f, 1.0f-0.0625f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
	{(vect3Df_s){-0.5f*CURSOR_SCALEFACT, 0.5f*CURSOR_SCALEFACT, -0.5f*CURSOR_SCALEFACT}, (float[]){0.125f, 1.0f-0.125f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
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

	cursorTexture=(u32*)linearAlloc(cursor_bin_size);
	memcpy(cursorTexture, cursor_bin, cursor_bin_size);
	GSPGPU_FlushDataCache(NULL, (u8*)cursorTexture, cursor_bin_size);
}

void drawCursor(cursor_s* c)
{
	if(!c || !c->active)return;

	// GPU_SetTexture(GPU_TEXUNIT0, (u32*)osConvertVirtToPhys((u32)cursorTexture),64,64,0x6,GPU_RGBA8);

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
			case 3:
				gsRotateX(M_PI);
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
