#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <3ds.h>

#include "math.h"
#include "gs.h"
#include "sky.h"
#include "world.h"
#include "dispatcher.h"
#include "player.h"
#include "text.h"
#include "configuration.h"

#include "test_vsh_shbin.h"
#include "terrain_bin.h"

#define TICKS_PER_VBL (268123480/60)

DVLB_s* shader;
u32* texData;

player_s player;
world_s world;

u32* gpuOut=(u32*)0x1F119400;
u32* gpuDOut=(u32*)0x1F370800;

u32 debugValue[128];

//stolen from staplebutt
void GPU_SetDummyTexEnv(u8 num)
{
	GPU_SetTexEnv(num, 
		GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0), 
		GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_REPLACE, 
		GPU_REPLACE, 
		0xFFFFFFFF);
}

// topscreen
void doFrame1()
{
	GPU_SetViewport((u32*)osConvertVirtToPhys((u32)gpuDOut),(u32*)osConvertVirtToPhys((u32)gpuOut),0,0,240*2,400);
	
	GPU_DepthRange(-1.0f, 0.0f);
	GPU_SetFaceCulling(GPU_CULL_BACK_CCW);
	GPU_SetStencilTest(false, GPU_ALWAYS, 0x00, 0xFF, 0x00);
	GPU_SetStencilOp(GPU_KEEP, GPU_KEEP, GPU_KEEP);
	GPU_SetBlendingColor(0,0,0,0);
	GPU_SetDepthTestAndWriteMask(true, GPU_GREATER, GPU_WRITE_ALL);
	
	GPUCMD_AddSingleParam(0x00010062, 0); 
	GPUCMD_AddSingleParam(0x000F0118, 0);
	
	//setup shader
		SHDR_UseProgram(shader, 0);
	
	GPU_SetAlphaBlending(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
	GPU_SetAlphaTest(false, GPU_ALWAYS, 0x00);
	
	GPU_SetTextureEnable(GPU_TEXUNIT0);
	
	GPU_SetTexEnv(0, 
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR), 
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
		GPU_TEVOPERANDS(0,0,0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_MODULATE, GPU_MODULATE, 
		0xFFFFFFFF);
	GPU_SetDummyTexEnv(1);
	GPU_SetDummyTexEnv(2);
	GPU_SetDummyTexEnv(3);
	GPU_SetDummyTexEnv(4);
	GPU_SetDummyTexEnv(5);

	//texturing stuff
		GPU_SetTexture(GPU_TEXUNIT0, (u32*)osConvertVirtToPhys((u32)texData),256,256,GPU_TEXTURE_MAG_FILTER(GPU_NEAREST)|GPU_TEXTURE_MIN_FILTER(GPU_NEAREST),GPU_RGBA8);
		GPU_SetAttributeBuffers(3, (u32*)osConvertVirtToPhys((u32)texData),
			GPU_ATTRIBFMT(0, 3, GPU_FLOAT)|GPU_ATTRIBFMT(1, 2, GPU_FLOAT)|GPU_ATTRIBFMT(2, 3, GPU_FLOAT),
			0xFFC, 0x210, 1, (u32[]){0x00000000}, (u64[]){0x210}, (u8[]){3});

	//draw world
		gsMatrixMode(GS_MODELVIEW);
		gsPushMatrix();
			setCameraPlayer(&player);
		// debugValue[5]=0;
		// debugValue[6]=0;
			drawWorld(&world, &player.camera);
		// debugValue[6]=1;
			drawCursor(&player.cursor);
			drawSky();
		gsPopMatrix();
	GPU_FinishDrawing();
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

size_t getMemFree();

void drawBottom()
{
	memset(gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL), 0x00, 240*320*3);
	static char str[256];
	sprintf(str, "3dscraft debug : %dKB LINEAR, %dKB REGULAR", (int)linearSpaceFree()/1024, (int)getMemFree()/1024);
	gfxDrawText(GFX_BOTTOM, GFX_LEFT, NULL, str, 240-fontDefault.height, 0);
	int i = countLines(superStr);
	while(i>240/fontDefault.height-1){cutLine(superStr);i--;}
	gfxDrawText(GFX_BOTTOM, GFX_LEFT, NULL, superStr, 240-fontDefault.height*2, 0);

	// gfxFlushBuffers();
	// gfxSwapBuffers();
}

extern u32* gxCmdBuf;

int main(int argc, char** argv)
{
	srvInit();	
	aptInit();
	gfxInit();
	fsInit();
	hidInit(NULL);
	irrstInit(NULL);

	initConfiguration(NULL, argc, argv);

	memset(debugValue, 0x00, sizeof(debugValue));

	aptOpenSession();
	print("%08X\n",(unsigned int)APT_SetAppCpuTimeLimit(NULL, 30));
	aptCloseSession();
	
	GPU_Init(NULL);

	gsInit();

	u32 gpuCmdSize=0x40000;
	u32* gpuCmd=(u32*)linearAlloc(gpuCmdSize*4);
	u32* gpuCmdRight=(u32*)linearAlloc(gpuCmdSize*4);

	GPU_Reset(gxCmdBuf, gpuCmd, gpuCmdSize);

	texData=(u32*)linearMemAlign(terrain_bin_size, 0x80); //textures need to be 0x80-byte aligned
	memcpy(texData, terrain_bin, terrain_bin_size);
	GSPGPU_FlushDataCache(NULL, (u8*)texData, terrain_bin_size);

	print("welcome\n");
	initDispatcher(NULL);
	initChunkPool();
	initWorld(&world);
	print("generating world...\n");

	initPlayer(&player);
	initSky();

	shader=SHDR_ParseSHBIN((u32*)test_vsh_shbin,test_vsh_shbin_size);

	GX_SetMemoryFill(gxCmdBuf, (u32*)gpuOut, 0x68B0D8FF, (u32*)&gpuOut[0x2EE00], 0x201, (u32*)gpuDOut, 0x00000000, (u32*)&gpuDOut[0x2EE00], 0x201);
	// gspWaitForPSC0();
	gfxSwapBuffersGpu();

	while(aptMainLoop())
	{
		// u64 val=svcGetSystemTick();

		hidScanInput();
		if(keysDown()&KEY_START)break;
		controlsPlayer(&player, &world);

		updatePlayer(&player, &world);
		updateWorld(&world);
		updateSky();
		updateDispatcher(NULL);

		gsStartFrame();
		doFrame1();
		GPUCMD_Finalize();
		GPUCMD_FlushAndRun(gxCmdBuf);

		//while GPU starts drawing the left buffer, we generate the right one
		GPUCMD_SetBuffer(gpuCmdRight, gpuCmdSize, 0);
		mtx44 m; loadIdentity44((float*)m);
		gsAdjustBufferMatrices(m);

		//we wait for the left buffer to finish drawing
		gspWaitForP3D();
		GX_SetDisplayTransfer(gxCmdBuf, (u32*)gpuOut, 0x019001E0, (u32*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 0x019001E0, 0x01001000);
		gspWaitForPPF();

		//we draw the right buffer, wait for it to finish and then switch back to left one
		GX_SetMemoryFill(gxCmdBuf, (u32*)gpuOut, 0x68B0D8FF, (u32*)&gpuOut[0x2EE00], 0x201, (u32*)gpuDOut, 0x00000000, (u32*)&gpuDOut[0x2EE00], 0x201);
		gspWaitForPSC0();
		GPUCMD_FlushAndRun(gxCmdBuf);
		gspWaitForP3D();
		GX_SetDisplayTransfer(gxCmdBuf, (u32*)gpuOut, 0x019001E0, (u32*)gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL), 0x019001E0, 0x01001000);
		gspWaitForPPF();
		GPUCMD_SetBuffer(gpuCmd, gpuCmdSize, 0);

		GX_SetMemoryFill(gxCmdBuf, (u32*)gpuOut, 0x68B0D8FF, (u32*)&gpuOut[0x2EE00], 0x201, (u32*)gpuDOut, 0x00000000, (u32*)&gpuDOut[0x2EE00], 0x201);
		gspWaitForPSC0();
		gfxSwapBuffersGpu();

		gspWaitForEvent(GSPEVENT_VBlank0, true);
		// debugValue[2]=(u32)(svcGetSystemTick()-val);

		// u64 val=svcGetSystemTick();
		// debugValue[1]=(u32)(svcGetSystemTick()-val);
		// print("%d\n", (int)debugValue[7]);
		// print("avg %d ticks (%d)\n", (int)(debugValue[5]/debugValue[6]), debugValue[6]);
		// print("drawing %d chunks... (%f vs %f)\n", (int)debugValue[0], (float)(debugValue[1]*100)/TICKS_PER_VBL, (float)(debugValue[2]*100)/TICKS_PER_VBL);
		// debugValue[0]=0;
		// u64 val=svcGetSystemTick();
		// debugValue[5]+=(u32)(svcGetSystemTick()-val);
		// debugValue[6]++;
		// drawBottom(); //DEBUG
	}

	exitDispatcher(NULL);
	exitWorld(&world);

	gsExit();
	irrstExit();
	hidExit();
	fsExit();
	gfxExit();
	aptExit();
	srvExit();
	return 0;
}
