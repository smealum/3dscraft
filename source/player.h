#ifndef PLAYER_H
#define PLAYER_H

#include <3ds/types.h>
#include "math.h"
#include "world.h"
#include "camera.h"

typedef struct
{
	vect3Df_s velocity;
	camera_s camera;
}player_s;

void initPlayer(player_s* p);
void controlsPlayer(player_s* p, world_s* w);
void updatePlayer(player_s* p, world_s* w);
void setCameraPlayer(player_s* p);

#endif
