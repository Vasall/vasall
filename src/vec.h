#ifndef VASALL_VEC_H
#define VASALL_VEC_H

#define M_PI 3.141592654
#define TO_RADS 3.141592654/180.0

typedef struct Vec3 {
	float x;
	float y;
	float z;
} Vec3;

/* Create a new 3d-vector */
Vec3 vecCreate(float x, float y, float z);

/* Set the values of a 3d-vector */
void vecSet(Vec3 *v, float x, float y, float z);

/* Copy the values from the second vector into the first */
void vecCpy(Vec3 *v1, Vec3 *v2);

/* Add two vectors and write to first vector */
void vecAdd(Vec3 *v1, Vec3 v2);

/* Add two vectors and return the result */
Vec3 vecAddRet(Vec3 v1, Vec3 v2);

/* Subtract one vector from another and write to first vector */
void vecSub(Vec3 *v1, Vec3 v2);

/* Subtract one vector from another and return the result */
Vec3 vecSubRet(Vec3 v1, Vec3 v2);

/* Scale a vector by a factor and write to the vector */
void vecScl(Vec3 *v, float f);

/* Scale a vector by a factor and return the result */
Vec3 vecSclRet(Vec3 v, float f);

/* Divide the vector by the factor */
void vecInvScl(Vec3 *v, float f);

/* Get the magnitude of a vector and return the result */
float vecMag(Vec3 v);

/* Normalize a vector and write to the vector */
void vecNrm(Vec3 *v);

/* Normalize a vector and return the result */
Vec3 vecNrmRet(Vec3 v);

/* Calculate the cross-product and return the result */
Vec3 vecCross(Vec3 v1, Vec3 v2);

typedef float *Mat4;

/* Create a new 4x4 matrix filled with zeros */
Mat4 mat4Zero(void);

/* Create an identity 4x4 matrix */
Mat4 mat4Idt(void);

/* Multiply two matrices and write result to first one */
void mat4Mult(Mat4 m1, Mat4 m2);

/* Multiply two matrices and return the result */
Mat4 mat4MultRet(Mat4 m1, Mat4 m2);

/* Combine two matrices into one */
void mat4Combine(Mat4 m1, Mat4 m2, Mat4 dst);

/* Display a 4x4 matrix in the console */
void mat4Print(Mat4 m);

#endif
