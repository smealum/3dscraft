#include <3ds.h>
#include "player.h"
#include "gs.h"

void initPlayer(player_s* p)
{
	if(!p)return;

	p->position=vect3Df(0.0f, 0.0f, 0.0f);
	p->velocity=vect3Df(0.0f, 0.0f, 0.0f);
	loadIdentity44((float*)p->orientation);
}

void controlsPlayer(player_s* p)
{
	if(!p)return;

	const u32 PAD=hidKeysHeld();

	vect3Df_s vx=vnormf(getMatrixColumn((float*)p->orientation, 0));
	vect3Df_s vz=vnormf(getMatrixColumn((float*)p->orientation, 2));

	if(PAD&KEY_UP)p->velocity=vaddf(p->velocity, vmulf(vz, 0.01f));
	if(PAD&KEY_DOWN)p->velocity=vaddf(p->velocity, vmulf(vz, -0.01f));
	if(PAD&KEY_RIGHT)p->velocity=vaddf(p->velocity, vmulf(vx, 0.01f));
	if(PAD&KEY_LEFT)p->velocity=vaddf(p->velocity, vmulf(vx, -0.01f));

	if(PAD&KEY_X)rotateMatrixX((float*)p->orientation, 0.01f);
	if(PAD&KEY_B)rotateMatrixX((float*)p->orientation, -0.01f);
	if(PAD&KEY_A)rotateMatrixY((float*)p->orientation, 0.01f);
	if(PAD&KEY_Y)rotateMatrixY((float*)p->orientation, -0.01f);
}

void updatePlayer(player_s* p)
{
	if(!p)return;

	//collisions go here
	//gravity goes here
	p->position=vaddf(p->position, p->velocity);

	p->velocity=vect3Df(0.0f, 0.0f, 0.0f);
}

//should be run on GS_MODELVIEW
void setCameraPlayer(player_s* p)
{
	if(!p)return;

	gsMultMatrix((float*)p->orientation);
	gsTranslate(p->position.x, p->position.y, p->position.z);
}
