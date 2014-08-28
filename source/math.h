#ifndef MATH_H
#define MATH_H

#include <3ds/types.h>
#include <math.h>

typedef float mtx44[4][4];
typedef float mtx33[3][3];

typedef struct
{
	s32 x, y, z;
}vect3Di_s;

static inline vect3Di_s vect3Di(s32 x, s32 y, s32 z)
{
	return (vect3Di_s){x,y,z};
}

static inline vect3Di_s vaddi(vect3Di_s u, vect3Di_s v)
{
	return (vect3Di_s){u.x+v.x,u.y+v.y,u.z+v.z};
}

static inline vect3Di_s vmuli(vect3Di_s v, s32 f)
{
	return (vect3Di_s){v.x*f,v.y*f,v.z*f};
}

typedef struct
{
	float x, y, z;
}vect3Df_s;

static inline vect3Df_s vect3Df(float x, float y, float z)
{
	return (vect3Df_s){x,y,z};
}

static inline vect3Df_s vaddf(vect3Df_s u, vect3Df_s v)
{
	return (vect3Df_s){u.x+v.x,u.y+v.y,u.z+v.z};
}

static inline vect3Df_s vmulf(vect3Df_s v, float f)
{
	return (vect3Df_s){v.x*f,v.y*f,v.z*f};
}

static inline vect3Df_s vnormf(vect3Df_s v)
{
	const float l=sqrtf(v.x*v.x+v.y*v.y+v.z*v.z);
	return (vect3Df_s){v.x/l,v.y/l,v.z/l};
}

static inline vect3Df_s vi2f(vect3Di_s v)
{
	return (vect3Df_s){(float)v.x,(float)v.y,(float)v.z};
}

void loadIdentity44(float* m);
void multMatrix44(float* m1, float* m2, float* m);

void translateMatrix(float* tm, float x, float y, float z);
void rotateMatrixX(float* tm, float x);
void rotateMatrixY(float* tm, float x);
void rotateMatrixZ(float* tm, float x);
void scaleMatrix(float* tm, float x, float y, float z);

void initProjectionMatrix(float* m, float fovy, float aspect, float near, float far);

vect3Df_s getMatrixColumn(float* m, u8 i);
vect3Df_s getMatrixRow(float* m, u8 i);

#endif
