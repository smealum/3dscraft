#include <3ds.h>

#include "camera.h"
#include "gs.h"

void initCamera(camera_s* c)
{
	if(!c)return;

	initProjectionMatrix((float*)c->projection, 1.3962634f, 240.0f/400.0f, 0.01f, 10.0f);
	loadIdentity44((float*)c->orientation);
	c->position=vect3Df(0.0f, 0.0f, 0.0f);
}

void setCamera(camera_s* c)
{
	if(!c)return;

	gsMatrixMode(GS_PROJECTION);
	gsLoadIdentity();
	gsMultMatrix((float*)c->projection);

	gsMatrixMode(GS_MODELVIEW);
	gsMultMatrix((float*)c->orientation);
	gsTranslate(-c->position.x, -c->position.y, -c->position.z);
}
