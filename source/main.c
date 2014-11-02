#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <3ds.h>

#include "math.h"
#include "gs.h"
#include "world.h"
#include "dispatcher.h"
#include "player.h"
#include "text.h"
#include "test_vsh_shbin.h"
#include "terrain_bin.h"

DVLB_s* shader;
u32* texData;

player_s player;
world_s world;
dispatcher_s dispatcher;

u32* gpuOut=(u32*)0x1F119400;
u32* gpuDOut=(u32*)0x1F370800;

u32 debugValue[128];

// topscreen
void doFrame1()
{
	//general setup
		GPU_SetViewport((u32*)osConvertVirtToPhys((u32)gpuDOut),(u32*)osConvertVirtToPhys((u32)gpuOut),0,0,240*2,400);

		GPU_DepthRange(-1.0f, 0.0f);

		GPU_SetFaceCulling(GPU_CULL_BACK_CCW);
		GPU_SetStencilTest(false, GPU_ALWAYS, 0x00, 0x00, 0x00);
		GPU_SetDepthTestAndWriteMask(true, GPU_GREATER, GPU_WRITE_ALL);

	// ?
		GPUCMD_AddSingleParam(0x00010062, 0x00000000); //param always 0x0 according to code
		GPUCMD_AddSingleParam(0x000F0118, 0x00000000);

	//setup shader
		SHDR_UseProgram(shader, 0);

	//?
		GPUCMD_AddSingleParam(0x000F0100, 0x00E40100);
		GPUCMD_AddSingleParam(0x000F0101, 0x01010000);
		GPUCMD_AddSingleParam(0x000F0104, 0x00000010);
	
	//texturing stuff
		GPUCMD_AddSingleParam(0x0002006F, 0x00000100);
		GPUCMD_AddSingleParam(0x000F0080, 0x00011001); //enables/disables texturing
	
	//texenv
		GPU_SetTexEnv(3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000);
		GPU_SetTexEnv(4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000);
		GPU_SetTexEnv(5, GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR), GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
			GPU_TEVOPERANDS(0,0,0), GPU_TEVOPERANDS(0,0,0), GPU_MODULATE, GPU_MODULATE, 0xFFFFFFFF);

	//texturing stuff
		GPU_SetTexture(GPU_TEXUNIT0, (u32*)osConvertVirtToPhys((u32)texData),256,256,0x6,GPU_RGBA8);

	//draw world
		gsMatrixMode(GS_MODELVIEW);
		gsPushMatrix();
			setCameraPlayer(&player);
			drawWorld(&world, &player.camera);
		gsPopMatrix();
}

char superStr[4096];

int countLines(char* str)
{
	if(!str)return 0;
	int cnt; for(cnt=1;*str=='\n'?++cnt:*str;str++);
	return cnt;
}

void cutLine(char* str)
{
	if(!str || !*str)return;
	char* str2=str;	for(;*str2&&*(str2+1)&&*str2!='\n';str2++);	str2++;
	memmove(str,str2,strlen(str2)+1);
}

void drawBottom()
{
	memset(gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL), 0x00, 240*320*3);
	gfxDrawText(GFX_BOTTOM, GFX_LEFT, NULL, "3dscraft debug", 240-fontDefault.height, 0);
	int i = countLines(superStr);
	while(i>240/fontDefault.height-1){cutLine(superStr);i--;}
	gfxDrawText(GFX_BOTTOM, GFX_LEFT, NULL, superStr, 240-fontDefault.height*2, 0);
}

extern u32* gxCmdBuf;

int main()
{
	srvInit();	
	aptInit();
	gfxInit();
	hidInit(NULL);
	irrstInit(NULL);
	
	GPU_Init(NULL);

	gsInit();

	u32 gpuCmdSize=0x40000;
	u32* gpuCmd=(u32*)linearAlloc(gpuCmdSize*4);

	GPU_Reset(gxCmdBuf, gpuCmd, gpuCmdSize);

	texData=(u32*)linearAlloc(terrain_bin_size);
	memcpy(texData, terrain_bin, terrain_bin_size);
	GSPGPU_FlushDataCache(NULL, (u8*)texData, terrain_bin_size);

	print("welcome\n");
	initDispatcher(&dispatcher);
	initChunkPool();
	initWorld(&world);
	print("generating world...\n");

	initPlayer(&player);

	shader=SHDR_ParseSHBIN((u32*)test_vsh_shbin,test_vsh_shbin_size);

	GX_SetMemoryFill(gxCmdBuf, (u32*)gpuOut, 0x404040FF, (u32*)&gpuOut[0x2EE00], 0x201, (u32*)gpuDOut, 0x00000000, (u32*)&gpuDOut[0x2EE00], 0x201);
	// gspWaitForPSC0();
	gfxSwapBuffersGpu();

	APP_STATUS status;
	while((status=aptGetStatus())!=APP_EXITING)
	{
		if(status==APP_RUNNING)
		{
			hidScanInput();
			if(keysDown()&KEY_START)break;
			controlsPlayer(&player);

			if(keysDown()&KEY_A)translateWorld(&world, vect3Di(1,0,0));
			if(keysDown()&KEY_B)translateWorld(&world, vect3Di(-1,0,0));

			updatePlayer(&player);
			updateWorld(&world);

			print("drawing %d chunks...\n", (int)debugValue[0]);
			debugValue[0]=0;

			GPUCMD_SetBuffer(gpuCmd, gpuCmdSize, 0);
			doFrame1();
			GPUCMD_Finalize();
			GPUCMD_Run(gxCmdBuf);
			gspWaitForP3D();

			GX_SetDisplayTransfer(gxCmdBuf, (u32*)gpuOut, 0x019001E0, (u32*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 0x019001E0, 0x01001000);
			gspWaitForPPF();

			GX_SetMemoryFill(gxCmdBuf, (u32*)gpuOut, 0x404040FF, (u32*)&gpuOut[0x2EE00], 0x201, (u32*)gpuDOut, 0x00000000, (u32*)&gpuDOut[0x2EE00], 0x201);
			gspWaitForPSC0();
			gfxSwapBuffersGpu();
		}
		gspWaitForEvent(GSPEVENT_VBlank0, true);
	}

	exitDispatcher(&dispatcher);
	gsExit();
	irrstExit();
	hidExit();
	gfxExit();
	aptExit();
	srvExit();
	return 0;
}
