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
	{(vect3Df_s){-0.5f, 0.0f, -0.5f}, (float[]){0.0f, 0.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
	{(vect3Df_s){+0.5f, 0.0f, -0.5f}, (float[]){1.0f, 0.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
	{(vect3Df_s){+0.5f, 0.0f, +0.5f}, (float[]){1.0f, 1.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
	//first triangle
	{(vect3Df_s){+0.5f, 0.0f, +0.5f}, (float[]){1.0f, 1.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
	{(vect3Df_s){-0.5f, 0.0f, +0.5f}, (float[]){0.0f, 1.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
	{(vect3Df_s){-0.5f, 0.0f, -0.5f}, (float[]){0.0f, 0.0f}, (vect3Df_s){1.0f, 0.0f, 0.0f}},
};

u32* cloudTexture;
gsVbo_s cloudVbo;

void initSky(void)
{
	gsVboInit(&cloudVbo);
	gsVboCreate(&cloudVbo, sizeof(cloudVboData));
	gsVboAddData(&cloudVbo, (void*)cloudVboData, sizeof(cloudVboData), sizeof(cloudVboData)/sizeof(faceVertex_s));
	gsVboFlushData(&cloudVbo);

	cloudTexture=(u32*)linearAlloc(cloud_bin_size);
	memcpy(cloudTexture, cloud_bin, cloud_bin_size);
	GSPGPU_FlushDataCache(NULL, (u8*)cloudTexture, cloud_bin_size);
}

void drawSky(void)
{
	gsPushMatrix();
		gsTranslate(0.5f, CLUSTER_SIZE*CHUNK_HEIGHT, 0.5f);
		gsScale(WORLD_SIZE*CLUSTER_SIZE*4,1.0f,WORLD_SIZE*CLUSTER_SIZE*4);
		gsVboDraw(&cloudVbo);
	gsPopMatrix();
}
