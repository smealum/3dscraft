#ifndef CAMERA_H
#define CAMERA_H

#include "math.h"

typedef struct
{
	mtx44 projection;
	mtx44 orientation;
	mtx44 modelview[2]; //modelview = orientation*translate(position) + left/right adjustments for 3D
	vect3Df_s position;
	vect4Df_s frustumPlane[6];
}camera_s;

void initCamera(camera_s* c);
void updateCamera(camera_s* c);
void setCamera(camera_s* c);

bool pointInCameraFrustum(camera_s* c, vect3Df_s pt);
bool aabbInCameraFrustum(camera_s* c, vect3Df_s o, vect3Df_s s, int planes);

#endif
