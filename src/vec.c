#include "vec.h"
#include "mat.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


extern void vec2_set(vec2_t v, float x, float y)
{
	v[0] = x;
	v[1] = y;
}

extern void vec2_clr(vec2_t v)
{
	memset(v, 0, VEC2_SIZE);
}

extern void vec2_cpy(vec2_t out, vec2_t in)
{
	memcpy(out, in, VEC2_SIZE);
}

extern int vec2_cmp(vec2_t v1, vec2_t v2)
{
	if(v1[0] != v2[0] || v1[1] != v2[1])
		return 0;

	return 1;
}

extern void vec2_add(vec2_t v1, vec2_t v2, vec2_t out)
{
	out[0] = v1[0] + v2[0];
	out[1] = v1[1] + v2[1];
}

extern void vec2_sub(vec2_t v1, vec2_t v2, vec2_t out)
{
	out[0] = v1[0] - v2[0];
	out[1] = v1[1] - v2[1];
}

extern void vec2_scl(vec2_t v1, float f, vec2_t out)
{
	out[0] = v1[0] * f;
	out[1] = v1[1] * f;
}

extern float vec2_mag(vec2_t v)
{
	double len = (v[0] * v[0]) + (v[1] * v[1]);
	return (float)sqrt(len);
}

extern void vec2_nrm(vec2_t in, vec2_t out)
{
	float len = vec2_mag(in);
	if(len == 0.0) {
		out[0] = 0.0;
		out[1] = 0.0;
		return;
	}

	out[0] = in[0] / len;
	out[1] = in[1] / len;
}

extern void vec2_print(vec2_t v)
{
	printf("%.2f/%.2f", v[0], v[1]);
}



extern void vec3_set(vec3_t v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

extern void vec3_clr(vec3_t v)
{
	memset(v, 0, VEC3_SIZE);
}

extern void vec3_cpy(vec3_t out, vec3_t in)
{
	memcpy(out, in, VEC3_SIZE);
}

extern int vec3_cmp(vec3_t v1, vec3_t v2)
{
	if(v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2])
		return 0;

	return 1;
}

extern void vec3_add(vec3_t v1, vec3_t v2, vec3_t res)
{
	res[0] = v1[0] + v2[0];
	res[1] = v1[1] + v2[1];
	res[2] = v1[2] + v2[2];
}

extern void vec3_sub(vec3_t v1, vec3_t v2, vec3_t res)
{
	res[0] = v1[0] - v2[0];
	res[1] = v1[1] - v2[1];
	res[2] = v1[2] - v2[2];
}

extern void vec3_scl(vec3_t v, float f, vec3_t res)
{
	res[0] = v[0] * f;
	res[1] = v[1] * f;
	res[2] = v[2] * f;
}

extern void vec3_inv_scl(vec3_t v, float f, vec3_t res)
{
	res[0] = v[0] / f;
	res[1] = v[1] / f;
	res[2] = v[2] / f;
}

extern float vec3_mag(vec3_t v)
{
	double len = (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
	return (float)sqrt(len);
}

extern void vec3_nrm(vec3_t v, vec3_t res)
{
	float len = vec3_mag(v);
	if(len == 0.0) {
		res[0] = 0.0;
		res[1] = 0.0;
		res[2] = 0.0;
		return;
	}

	res[0] = v[0] / len;
	res[1] = v[1] / len;
	res[2] = v[2] / len;
}

extern float vec3_dot(vec3_t v1, vec3_t v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

extern void vec3_cross(vec3_t v1, vec3_t v2, vec3_t res)
{
	res[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	res[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	res[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

extern void vec3_rot_x(vec3_t v, float angle, vec3_t res)
{
	mat3_t rmat;
	mat3_idt(rmat);
	rmat[0x4] = cos(angle);
	rmat[0x5] = -sin(angle);
	rmat[0x7] = sin(angle);
	rmat[0x8] = cos(angle);

	vec3_trans(v, rmat, res);
}

extern void vec3_rot_y(vec3_t v, float angle, vec3_t res)
{
	mat3_t rmat;
	mat3_idt(rmat);
	rmat[0x0] = cos(angle);
	rmat[0x2] = sin(angle);
	rmat[0x6] = -sin(angle);
	rmat[0x8] = cos(angle);

	vec3_trans(v, rmat, res);
}

extern void vec3_rot_z(vec3_t v, float angle, vec3_t res)
{
	mat3_t rmat;
	mat3_idt(rmat);
	rmat[0x0] = cos(angle);
	rmat[0x1] = -sin(angle);
	rmat[0x3] = sin(angle);
	rmat[0x4] = cos(angle);
	vec3_trans(v, rmat, res);
}

extern void vec3_rot_axes(vec3_t v, float angle, vec3_t axis, vec3_t res)
{
	mat3_t rmat;
	float q0, q1, q2, q3;

	vec3_nrm(axis, axis);
	q0 = cos(angle / 2);
	q1 = sin(angle / 2) * axis[0];
	q2 = sin(angle / 2) * axis[1];
	q3 = sin(angle / 2) * axis[2];

	rmat[0x0] = q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3;
	rmat[0x1] = 2 * (q1 * q2 - q0 * q3);
	rmat[0x2] = 2 * (q1 * q3 + q0 * q2);
	rmat[0x3] = 2 * (q2 * q1 + q0 * q3);
	rmat[0x4] = q0 * q0 - q1 * q1 + q2 * q2 - q3 * q3;
	rmat[0x5] = 2 * (q2 * q3 - q0 * q1);
	rmat[0x6] = 2 * (q3 * q1 - q0 * q2);
	rmat[0x7] = 2 * (q3 * q2 + q0 * q1);
	rmat[0x8] = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;

	vec3_trans(v, rmat, res);
}

extern void vec3_trans(vec3_t v, mat3_t mat, vec3_t res)
{
	vec3_t tmp;
	vec3_cpy(tmp, v);

	res[0] = tmp[0] * mat[0x0] + tmp[1] * mat[0x1] + tmp[2] * mat[0x2];
	res[1] = tmp[0] * mat[0x3] + tmp[1] * mat[0x4] + tmp[2] * mat[0x5];
	res[2] = tmp[0] * mat[0x6] + tmp[1] * mat[0x7] + tmp[2] * mat[0x8];
}

extern void vec3_print(vec3_t v)
{
	printf("%.2f/%.2f/%.2f", v[0], v[1], v[2]);
}

extern float vec3_barry_centric(vec3_t p1, vec3_t p2, vec3_t p3, vec2_t pos)
{
	float det, l1, l2, l3;

	det = (p2[2] - p3[2]) * (p1[0] - p3[0]) + (p3[0] - p2[0]) * (p1[2] - p3[2]);
	l1 = ((p2[2] - p3[2]) * (pos[0] - p3[0]) + (p3[0] - p2[0]) * (pos[1] - p3[2])) / det;
	l2 = ((p3[2] - p1[2]) * (pos[0] - p3[0]) + (p1[0] - p3[0]) * (pos[1] - p3[2])) / det;
	l3 = 1.0 - l1 - l2;

	return (l1 * p1[1]) + (l2 * p2[1]) + (l3 * p3[1]);
}
