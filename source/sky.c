#include <3ds.h>

#include "math.h"
#include "block.h"
#include "world.h"
#include "gs.h"
#include "sky.h"

#include "cloud_bin.h"

const faceVertex_s cloudVboData[]=
{
	//first triangle
	{(vect3Df_s){-0.5f, 0.0f, -0.5f}, (float[]){0.0f, 0.0f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
	{(vect3Df_s){+0.5f, 0.0f, -0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
	{(vect3Df_s){+0.5f, 0.0f, +0.5f}, (float[]){1.0f, 1.0f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
	//first triangle
	{(vect3Df_s){+0.5f, 0.0f, +0.5f}, (float[]){1.0f, 1.0f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
	{(vect3Df_s){-0.5f, 0.0f, +0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
	{(vect3Df_s){-0.5f, 0.0f, -0.5f}, (float[]){0.0f, 0.0f}, (vect3Df_s){0.0f, 1.0f, 0.0f}},
};

vect3Df_s cloudOffset;
vect3Df_s cloudSize;

u32* cloudTexture;
gsVbo_s cloudVbo;

void initSky(void)
{
	gsVboInit(&cloudVbo);
	gsVboCreate(&cloudVbo, sizeof(cloudVboData));
	gsVboAddData(&cloudVbo, (void*)cloudVboData, sizeof(cloudVboData), sizeof(cloudVboData)/sizeof(faceVertex_s));
	gsVboFlushData(&cloudVbo);

	cloudTexture=(u32*)linearMemAlign(cloud_bin_size, 0x80);
	if(cloudTexture)
	{
		memcpy(cloudTexture, cloud_bin, cloud_bin_size);
		GSPGPU_FlushDataCache(NULL, (u8*)cloudTexture, cloud_bin_size);
	}

	cloudOffset=vect3Df(0.0f, 0.0f, 0.0f);
	cloudSize=vect3Df(0.5f, 0.5f, 0.0f);
}

void updateSky(void)
{
	cloudOffset.x+=0.00001f;
	cloudOffset.y-=0.00003f;

	//update VBO texcoords
	faceVertex_s* vboData=(faceVertex_s*)cloudVbo.data;

	vboData[0].texcoord[0]=cloudOffset.x;
	vboData[0].texcoord[1]=cloudOffset.y;

	vboData[1].texcoord[0]=cloudOffset.x+cloudSize.x;
	vboData[1].texcoord[1]=cloudOffset.y;

	vboData[2].texcoord[0]=cloudOffset.x+cloudSize.x;
	vboData[2].texcoord[1]=cloudOffset.y+cloudSize.y;

	vboData[3].texcoord[0]=cloudOffset.x+cloudSize.x;
	vboData[3].texcoord[1]=cloudOffset.y+cloudSize.y;

	vboData[4].texcoord[0]=cloudOffset.x;
	vboData[4].texcoord[1]=cloudOffset.y+cloudSize.y;

	vboData[5].texcoord[0]=cloudOffset.x;
	vboData[5].texcoord[1]=cloudOffset.y;
}

void drawSky(void)
{
	GPU_SetTexture(GPU_TEXUNIT0, (u32*)osConvertVirtToPhys((u32)cloudTexture),256,256,GPU_TEXTURE_MAG_FILTER(GPU_NEAREST)|GPU_TEXTURE_MIN_FILTER(GPU_NEAREST)|GPU_TEXTURE_WRAP_S(GPU_REPEAT)|GPU_TEXTURE_WRAP_T(GPU_REPEAT),GPU_RGBA8);
	gsPushMatrix();
		gsTranslate(0.5f, CLUSTER_SIZE*CHUNK_HEIGHT, 0.5f);
		gsScale(WORLD_SIZE*CLUSTER_SIZE*8,1.0f,WORLD_SIZE*CLUSTER_SIZE*8);
		gsVboDraw(&cloudVbo);
	gsPopMatrix();
}
