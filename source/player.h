#ifndef PLAYER_H
#define PLAYER_H

#include <3ds/types.h>
#include "math.h"
#include "world.h"
#include "camera.h"
#include "cursor.h"

typedef struct
{
	vect3Df_s position;
	vect3Df_s velocity;
	vect3Df_s acceleration;
	vect3Df_s deltaOrientation;
	camera_s camera;
	cursor_s cursor;
	float headbob;
	bool destroy;
	bool flying;
	u8 block;
}player_s;

void initPlayer(player_s* p);
void controlsPlayer(player_s* p, world_s* w);
void updatePlayer(player_s* p, world_s* w, float timeDelta);
void setCameraPlayer(player_s* p);

#endif
