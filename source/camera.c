#include <3ds.h>

#include "camera.h"
#include "gs.h"

void initCamera(camera_s* c)
{
	if(!c)return;

	initProjectionMatrix((float*)c->projection, 1.3962634f, 240.0f/400.0f, 0.01f, 1000.0f);
	loadIdentity44((float*)c->orientation);
	c->position=vect3Df(0.0f, 0.0f, 0.0f);
}

void updateCameraFrustum(camera_s* c)
{
	if(!c)return;

	float final[4*4];
	multMatrix44((float*)c->projection, (float*)c->modelview[0], final);

	const vect4Df_s rowX = getMatrixColumn4(final, 0);
	const vect4Df_s rowY = getMatrixColumn4(final, 1);
	const vect4Df_s rowZ = getMatrixColumn4(final, 2);
	const vect4Df_s rowW = getMatrixColumn4(final, 3);

	//ordered by priority for culling
	c->frustumPlane[0] = vnormf4(vsubf4(rowW, rowZ)); //near plane
	c->frustumPlane[1] = vnormf4(vaddf4(rowW, rowX)); //right plane
	c->frustumPlane[2] = vnormf4(vsubf4(rowW, rowX)); //left plane
	c->frustumPlane[3] = vnormf4(vaddf4(rowW, rowY)); //top plane
	c->frustumPlane[4] = vnormf4(vsubf4(rowW, rowY)); //bottom plane
	c->frustumPlane[5] = vnormf4(vaddf4(rowW, rowZ)); //far plane
}

void updateCamera(camera_s* c)
{
	if(!c)return;

	memcpy(c->modelview[0], c->orientation, sizeof(mtx44));
	rotateMatrixZ((float*)c->modelview[0], M_PI/2, true); //because framebuffer is sideways...
	translateMatrix((float*)c->modelview[0], -c->position.x, -c->position.y, -c->position.z);

	updateCameraFrustum(c);
}

void setCamera(camera_s* c)
{
	if(!c)return;

	gsMatrixMode(GS_PROJECTION);
	gsLoadIdentity();
	gsMultMatrix((float*)c->projection);

	gsMatrixMode(GS_MODELVIEW);
	gsMultMatrix((float*)c->modelview[0]);
}

bool pointInCameraFrustum(camera_s* c, vect3Df_s pt)
{
	if(!c)return false;
	const vect4Df_s pt4=vect4Df(pt.x,pt.y,pt.z,1.0f);
	int i; for(i=0;i<6;i++)if(vdotf4(pt4,c->frustumPlane[i])<0.0f)return false;
	return true;
}

vect3Df_s box[]={(vect3Df_s){0.f,0.f,0.f},
				(vect3Df_s){1.f,0.f,0.f},
				(vect3Df_s){0.f,1.f,0.f},
				(vect3Df_s){0.f,0.f,1.f},
				(vect3Df_s){1.f,1.f,0.f},
				(vect3Df_s){1.f,0.f,1.f},
				(vect3Df_s){0.f,1.f,1.f},
				(vect3Df_s){1.f,1.f,1.f}};

//et "Assarsson and Moller report that they found no observable penalty in the rendering when skipping further tests"
bool aabbInCameraFrustum(camera_s* c, vect3Df_s o, vect3Df_s s, int planes)
{
	if(!c)return false;
	if(planes<=0 || planes>6)return false;
	int i, j;
	for(i=0;i<planes;i++)
	{
		int in=0, out=0;
		for(j=0;j<8 && (!in || !out);j++)
		{
			const vect3Df_s pt=vaddf(o,vscalef(box[j],s));
			if(vdotf4(vect4Df(pt.x,pt.y,pt.z,1.0f),c->frustumPlane[i])<0.0f)out++;
			else in++;
		}
		if(!in)return false;
	}
	return true;
}
