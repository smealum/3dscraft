#ifndef MATH_H

#include <3ds/types.h>

typedef struct
{
	s32 x, y, z;
}vect3Di_s;

static inline vect3Di_s vect3Di(s32 x, s32 y, s32 z)
{
	return (vect3Di_s){x,y,z};
}

typedef struct
{
	float x, y, z;
}vect3Df_s;

void loadIdentity44(float* m);
void multMatrix44(float* m1, float* m2, float* m);

void translateMatrix(float* tm, float x, float y, float z);
void rotateMatrixX(float* tm, float x);
void rotateMatrixZ(float* tm, float x);
void scaleMatrix(float* tm, float x, float y, float z);

void initProjectionMatrix(float* m, float fovy, float aspect, float near, float far);

#endif
