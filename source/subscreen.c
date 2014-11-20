#include <3ds.h>

#include "subscreen.h"
#include "player.h"
#include "block.h"

#include "blocks_bin.h"

#define BLOCK_SIZE (16)

//ALL OF THIS IS TEMPORARY
u8 previousBlock;
u8 buffersDrawn; //especially this

void initSubscreen()
{
	previousBlock=255;
	buffersDrawn=0;
}

//super suboptimal but ok because doesn't happen often
void drawBlockSoft(gfxScreen_t screen, gfx3dSide_t side, u8* blockData, s16 x, s16 y, u8 f, int id)
{
	//assume x and y are ok
	if(!blockData)return;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	int xOffset=(id%16), yOffset=(id/16);
	xOffset*=16; yOffset*=16;

	int i, j;
	for(j=y; j<y+(BLOCK_SIZE<<f); j++)
	{
		const int bj=(j-y)>>f;
		for(i=x; i<x+(BLOCK_SIZE<<f); i++)
		{
			const int bi=(i-x)>>f;
			const int v=((i)+(j)*fbWidth)*3;
			const int v2=((xOffset+bj)+(yOffset+BLOCK_SIZE-1-bi)*256)*4;
			fbAdr[v+0]=blockData[v2+2];
			fbAdr[v+1]=blockData[v2+1];
			fbAdr[v+2]=blockData[v2+0];
		}
	}
}

void drawSubscreen(player_s* p)
{
	if(!p)return;

	if(p->block!=previousBlock || !buffersDrawn)
	{
		//redraw subscreen
		memset(gfxGetFramebuffer(GFX_BOTTOM, GFX_BOTTOM, NULL, NULL), 0x00, 320*240*3);

		const u8 id=p->block;

		if(id>1)drawBlockSoft(GFX_BOTTOM, GFX_BOTTOM, (u8*)blocks_bin, 10, 16, 2, blockTypes[id-1].side);
		drawBlockSoft(GFX_BOTTOM, GFX_BOTTOM, (u8*)blocks_bin, 10, 96, 3, blockTypes[id].side);
		drawBlockSoft(GFX_BOTTOM, GFX_BOTTOM, (u8*)blocks_bin, 10, 96+128+16, 2, (id<BLOCK_NUM-1)?blockTypes[id+1].side:blockTypes[1].side);

		if(p->block!=previousBlock)buffersDrawn=0;
		else buffersDrawn++;
		previousBlock=p->block;
	}
}

void exitSubscreen()
{

}

