#ifndef CAMERA_H
#define CAMERA_H

#include "math.h"

typedef struct
{
	mtx44 projection;
	mtx44 orientation;
	vect3Df_s position;
	vect4Df_s frustumPlane[6];
}camera_s;

void initCamera(camera_s* c);
void setCamera(camera_s* c);

#endif
