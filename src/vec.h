#ifndef _VEC_H
#define _VEC_H

#include "mat.h"

#define M_PI 3.141592654
#define TO_RADS (3.14/180.0)

#define VEC2_SIZE (sizeof(float)*2)
typedef float vec2_t[2];

#define VEC3_SIZE (sizeof(float)*3)
typedef float vec3_t[3];

#define INT2 (sizeof(int)*2)
typedef int int2_t[2];

#define INT3 (sizeof(int)*3)
typedef int int3_t[3];

void vec3_set(vec3_t v, float x, float y, float z);
void vec3_cpy(vec3_t dst, vec3_t src);
void vec3_add(vec3_t v1, vec3_t v2, vec3_t res);
void vec3_sub(vec3_t v1, vec3_t v2, vec3_t res);
void vec3_scl(vec3_t v, float f, vec3_t res);
void vec3_inv_scl(vec3_t v, float f, vec3_t res);
float vec3_mag(vec3_t v);
void vec3_nrm(vec3_t v, vec3_t res);
float vec3_dot(vec3_t v1, vec3_t v2);
void vec3_cross(vec3_t v1, vec3_t v2, vec3_t res);
void vec3_rot_x(vec3_t v, float angle, vec3_t res);
void vec3_rot_y(vec3_t v, float angle, vec3_t res);
void vec3_rot_z(vec3_t v, float angle, vec3_t res);
void vec3_rot_axes(vec3_t v, float angle, vec3_t axis, vec3_t res);
void vec3_trans(vec3_t v, mat3_t mat, vec3_t res);
void vec3_dump(vec3_t v);
float vec3_barry_centric(vec3_t p1, vec3_t p2, vec3_t p3, vec2_t pos);

#endif
