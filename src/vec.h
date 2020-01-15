#ifndef VASALL_VEC_H
#define VASALL_VEC_H

#include "mat.h"

#define M_PI 3.141592654
#define TO_RADS 3.141592654/180.0

#define VEC3_SIZE (sizeof(float)*3)
typedef float Vec3[3];

/* Set the values of a 3d-vector */
void vecSet(Vec3 v, float x, float y, float z);

/* Copy the values from the second vector into the first */
void vecCpy(Vec3 dst, Vec3 src);

/* Add two vectors */
void vecAdd(Vec3 v1, Vec3 v2, Vec3 res);

/* Subtract one vector from another */
void vecSub(Vec3 v1, Vec3 v2, Vec3 res);

/* Scale a vector by a factor */
void vecScl(Vec3 v, float f, Vec3 res);

/* Rotate a vector around the x axis */
void vecRotX(Vec3 v, float angle, Vec3 res);

/* Rotate a vector around the y axis */
void vecRotY(Vec3 v, float angle, Vec3 res);

/* Rotate a vector around the z axis */
void vecRotZ(Vec3 v, float angle, Vec3 res);

/* Transform a vector using a 3x3 transformation matrix */
void vecTransf(Vec3 v, Mat3 mat, Vec3 res);

/* Divide the vector by the factor */
void vecInvScl(Vec3 v, float f, Vec3 res);

/* Get the magnitude of a vector and return the result */
float vecMag(Vec3 v);

/* Normalize a vector */
void vecNrm(Vec3 v, Vec3 res);

/* Calculate the cross-product */
void vecCross(Vec3 v1, Vec3 v2, Vec3 res);

/* Output a vector in the terminal */
void vecPrint(Vec3 v);

#endif
