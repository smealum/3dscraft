#include <3ds.h>
#include "player.h"
#include "world.h"
#include "gs.h"

void initPlayer(player_s* p)
{
	if(!p)return;

	initCamera(&p->camera);
	p->camera.position=vect3Df(0.0f, CHUNK_HEIGHT*CLUSTER_SIZE/2+CLUSTER_SIZE, 0.0f);
	loadIdentity44((float*)p->camera.orientation);
	rotateMatrixZ((float*)p->camera.orientation, M_PI/2, false);
	rotateMatrixY((float*)p->camera.orientation, M_PI, false);
	p->velocity=vect3Df(0.0f, 0.0f, 0.0f);
}

void controlsPlayer(player_s* p)
{
	if(!p)return;

	const u32 PAD=hidKeysHeld();
	circlePosition cstick;
	hidCstickRead(&cstick);

	vect3Df_s vx=vnormf(getMatrixColumn((float*)p->camera.orientation, 0));
	vect3Df_s vy=vnormf(getMatrixColumn((float*)p->camera.orientation, 1));
	vect3Df_s vz=vnormf(getMatrixColumn((float*)p->camera.orientation, 2));

	if(PAD&KEY_UP)p->velocity=vaddf(p->velocity, vmulf(vz, -1.0f));
	if(PAD&KEY_DOWN)p->velocity=vaddf(p->velocity, vmulf(vz, 1.0f));
	if(PAD&KEY_RIGHT)p->velocity=vaddf(p->velocity, vmulf(vy, -1.0f));
	if(PAD&KEY_LEFT)p->velocity=vaddf(p->velocity, vmulf(vy, 1.0f));
	if(PAD&KEY_R)p->velocity=vaddf(p->velocity, vmulf(vx, -1.0f));
	if(PAD&KEY_L)p->velocity=vaddf(p->velocity, vmulf(vx, 1.0f));

	// if(PAD&KEY_X)rotateMatrixX((float*)p->camera.orientation, 0.1f, false);
	// if(PAD&KEY_B)rotateMatrixX((float*)p->camera.orientation, -0.1f, false);
	// if(PAD&KEY_A)rotateMatrixY((float*)p->camera.orientation, 0.1f, false);
	// if(PAD&KEY_Y)rotateMatrixY((float*)p->camera.orientation, -0.1f, false);

	rotateMatrixY((float*)p->camera.orientation, (cstick.dx*0.2f)/0x9c, false);
}

void updatePlayerFrustum(player_s* p)
{
	if(!p)return;
}

void updatePlayer(player_s* p)
{
	if(!p)return;

	//collisions go here
	//gravity goes here
	p->camera.position=vaddf(p->camera.position, p->velocity);

	p->velocity=vect3Df(0.0f, 0.0f, 0.0f);
}

//should be run on GS_MODELVIEW
void setCameraPlayer(player_s* p)
{
	if(!p)return;

	setCamera(&p->camera);
}
