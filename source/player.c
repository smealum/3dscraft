#include <3ds.h>
#include "player.h"
#include "world.h"
#include "gs.h"

void initPlayer(player_s* p)
{
	if(!p)return;

	p->position=vect3Df(0.0f, CHUNK_HEIGHT*CLUSTER_SIZE/2+CLUSTER_SIZE, 0.0f);
	p->velocity=vect3Df(0.0f, 0.0f, 0.0f);
	loadIdentity44((float*)p->orientation);
	rotateMatrixZ((float*)p->orientation, M_PI/2, false);
	rotateMatrixY((float*)p->orientation, M_PI, false);
}

void controlsPlayer(player_s* p)
{
	if(!p)return;

	const u32 PAD=hidKeysHeld();

	vect3Df_s vx=vnormf(getMatrixColumn((float*)p->orientation, 0));
	vect3Df_s vy=vnormf(getMatrixColumn((float*)p->orientation, 1));
	vect3Df_s vz=vnormf(getMatrixColumn((float*)p->orientation, 2));

	if(PAD&KEY_UP)p->velocity=vaddf(p->velocity, vmulf(vz, -1.0f));
	if(PAD&KEY_DOWN)p->velocity=vaddf(p->velocity, vmulf(vz, 1.0f));
	if(PAD&KEY_RIGHT)p->velocity=vaddf(p->velocity, vmulf(vy, -1.0f));
	if(PAD&KEY_LEFT)p->velocity=vaddf(p->velocity, vmulf(vy, 1.0f));
	if(PAD&KEY_R)p->velocity=vaddf(p->velocity, vmulf(vx, -1.0f));
	if(PAD&KEY_L)p->velocity=vaddf(p->velocity, vmulf(vx, 1.0f));

	if(PAD&KEY_X)rotateMatrixX((float*)p->orientation, 0.1f, false);
	if(PAD&KEY_B)rotateMatrixX((float*)p->orientation, -0.1f, false);
	if(PAD&KEY_A)rotateMatrixY((float*)p->orientation, 0.1f, false);
	if(PAD&KEY_Y)rotateMatrixY((float*)p->orientation, -0.1f, false);
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
	gsTranslate(-p->position.x, -p->position.y, -p->position.z);
}
