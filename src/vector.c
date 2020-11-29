#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


extern void vec2_set(vec2_t out, float x, float y)
{
	out[0] = x;
	out[1] = y;
}

extern void vec2_clr(vec2_t in)
{
	memset(in, 0, VEC2_SIZE);
}

extern void vec2_cpy(vec2_t out, vec2_t in)
{
	memcpy(out, in, VEC2_SIZE);
}

extern int vec2_cmp(vec2_t in1, vec2_t in2)
{
	if(in1[0] != in2[0] || in1[1] != in2[1])
		return 0;

	return 1;
}

extern void vec2_add(vec2_t in1, vec2_t in2, vec2_t out)
{
	out[0] = in1[0] + in2[0];
	out[1] = in1[1] + in2[1];
}

extern void vec2_sub(vec2_t in1, vec2_t in2, vec2_t out)
{
	out[0] = in1[0] - in2[0];
	out[1] = in1[1] - in2[1];
}

extern void vec2_scl(vec2_t in, float f, vec2_t out)
{
	out[0] = in[0] * f;
	out[1] = in[1] * f;
}

extern float vec2_len(vec2_t in)
{
	double l = (in[0] * in[0]) + (in[1] * in[1]);
	return (float)sqrt(l);
}

extern void vec2_nrm(vec2_t in, vec2_t out)
{
	float len = vec2_len(in);
	if(len == 0.0) {
		out[0] = 0.0;
		out[1] = 0.0;
		return;
	}

	out[0] = in[0] / len;
	out[1] = in[1] / len;
}

extern void vec2_print(vec2_t in)
{
	printf("%.2f/%.2f", in[0], in[1]);
}



extern void vec3_set(vec3_t out, float x, float y, float z)
{
	out[0] = x;
	out[1] = y;
	out[2] = z;
}

extern void vec3_clr(vec3_t in)
{
	memset(in, 0, VEC3_SIZE);
}

extern void vec3_cpy(vec3_t out, vec3_t in)
{
	memcpy(out, in, VEC3_SIZE);
}

extern void vec3_cpy_v2(vec3_t out, vec2_t in)
{
	out[0] = in[0];
	out[1] = 0.0;
	out[2] = in[1];
}

extern int vec3_cmp(vec3_t in1, vec3_t in2)
{
	if(in1[0] != in2[0] || in1[1] != in2[1] || in1[2] != in2[2])
		return 0;

	return 1;
}

extern void vec3_add(vec3_t in1, vec3_t in2, vec3_t out)
{
	out[0] = in1[0] + in2[0];
	out[1] = in1[1] + in2[1];
	out[2] = in1[2] + in2[2];
}

extern void vec3_sub(vec3_t in1, vec3_t in2, vec3_t out)
{
	out[0] = in1[0] - in2[0];
	out[1] = in1[1] - in2[1];
	out[2] = in1[2] - in2[2];
}

extern void vec3_scl(vec3_t in, float f, vec3_t out)
{
	out[0] = in[0] * f;
	out[1] = in[1] * f;
	out[2] = in[2] * f;
}

extern void vec3_inv_scl(vec3_t in, float f, vec3_t out)
{
	out[0] = in[0] / f;
	out[1] = in[1] / f;
	out[2] = in[2] / f;
}

extern void vec3_mult(vec3_t in1, vec3_t in2, vec3_t out)
{
	vec3_t tmp;

	tmp[0] = in1[0] * in2[0];
	tmp[1] = in1[1] * in2[1];
	tmp[2] = in1[2] * in2[2];

	vec3_cpy(out, tmp);
}

extern void vec3_div(vec3_t in1, vec3_t in2, vec3_t out)
{
	vec3_t tmp;

	tmp[0] = in1[0] / in2[0];
	tmp[1] = in1[1] / in2[1];
	tmp[2] = in1[2] / in2[2];

	vec3_cpy(out, tmp);
}

extern float vec3_len(vec3_t in)
{
	double l = (in[0] * in[0]) + (in[1] * in[1]) + (in[2] * in[2]);
	return (float)sqrt(l);
}

extern float vec3_sqrlen(vec3_t in)
{
	return (in[0] * in[0]) + (in[1] * in[1]) + (in[2] * in[2]);
}

extern void vec3_nrm(vec3_t in, vec3_t out)
{
	float len = vec3_len(in);
	if(len == 0.0) {
		out[0] = 0.0;
		out[1] = 0.0;
		out[2] = 0.0;
		return;
	}

	out[0] = in[0] / len;
	out[1] = in[1] / len;
	out[2] = in[2] / len;
}

extern float vec3_dot(vec3_t v1, vec3_t v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

extern void vec3_cross(vec3_t v1, vec3_t v2, vec3_t out)
{
	out[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	out[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	out[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

extern void vec3_rot_x(vec3_t in, float angle, vec3_t out)
{
	mat3_t rmat;
	mat3_idt(rmat);
	rmat[0x0] =  1.0;
	rmat[0x4] =  cos(angle);
	rmat[0x5] = -sin(angle);
	rmat[0x7] =  sin(angle);
	rmat[0x8] =  cos(angle);
	vec3_trans(in, rmat, out);
}

extern void vec3_rot_y(vec3_t in, float angle, vec3_t out)
{
	mat3_t rmat;
	mat3_idt(rmat);
	rmat[0x0] =  cos(angle);
	rmat[0x2] =  sin(angle);
	rmat[0x3] =  1.0;
	rmat[0x6] = -sin(angle);
	rmat[0x8] =  cos(angle);
	vec3_trans(in, rmat, out);
}

extern void vec3_rot_z(vec3_t in, float angle, vec3_t out)
{
	mat3_t rmat;
	mat3_idt(rmat);
	rmat[0x0] =  cos(angle);
	rmat[0x1] = -sin(angle);
	rmat[0x3] =  sin(angle);
	rmat[0x4] =  cos(angle);
	rmat[0x8] =  1.0;
	vec3_trans(in, rmat, out);
}

extern void vec3_rot_axes(vec3_t in, float angle, vec3_t axis, vec3_t out)
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

	vec3_trans(in, rmat, out);
}

extern void vec3_trans(vec3_t in, mat3_t mat, vec3_t out)
{
	vec3_t tmp;
	vec3_cpy(tmp, in);

	out[0] = tmp[0] * mat[0x0] + tmp[1] * mat[0x1] + tmp[2] * mat[0x2];
	out[1] = tmp[0] * mat[0x3] + tmp[1] * mat[0x4] + tmp[2] * mat[0x5];
	out[2] = tmp[0] * mat[0x6] + tmp[1] * mat[0x7] + tmp[2] * mat[0x8];
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

extern void vec3_calc_nrm(vec3_t a, vec3_t b, vec3_t c, vec3_t nrm)
{
	vec3_t d1;
	vec3_t d2;

	vec3_sub(b, a, d1);
	vec3_sub(c, a, d2);

	vec3_cross(d1, d2, nrm);
	vec3_nrm(nrm, nrm);
}


extern void vec4_set(vec4_t out, float x, float y, float z, float w)
{
	out[0] = x;
	out[1] = y;
	out[2] = z;
	out[3] = w;
}

extern void vec4_clr(vec4_t in)
{
	memset(in, 0, VEC4_SIZE);
}

extern void vec4_cpy(vec4_t out, vec4_t in)
{
	memcpy(out, in, VEC4_SIZE);
}

extern int vec4_cmp(vec4_t in1, vec4_t in2)
{
	if(in1[0] != in2[0] || in1[1] != in2[1] || in1[2] != in2[2] || 
			in1[3] != in2[3])
		return 0;

	return 1;
}

extern void vec4_add(vec4_t in1, vec4_t in2, vec4_t out)
{
	out[0] = in1[0] + in2[0];
	out[1] = in1[1] + in2[1];
	out[2] = in1[2] + in2[2];
	out[3] = in1[3] + in2[3];
}

extern void vec4_sub(vec4_t in1, vec4_t in2, vec4_t out)
{
	out[0] = in1[0] - in2[0];
	out[1] = in1[1] - in2[1];
	out[2] = in1[2] - in2[2];
	out[3] = in1[3] - in2[3];
}

extern void vec4_scl(vec4_t in, float f, vec4_t out)
{
	out[0] = in[0] * f;
	out[1] = in[1] * f;
	out[2] = in[2] * f;
	out[3] = in[3] * f;
}

extern float vec4_len(vec4_t in)
{
	double l;
	l = (in[0] * in[0]) + (in[1] * in[1]) + (in[2] * in[2]) + (in[3] * in[3]);
	return (float)sqrt(l);
}

extern void vec4_nrm(vec4_t in, vec4_t out)
{
	float len = vec4_len(in);
	if(len == 0.0) {
		out[0] = 0.0;
		out[1] = 0.0;
		out[2] = 0.0;
		out[3] = 0.0;
		return;
	}

	out[0] = in[0] / len;
	out[1] = in[1] / len;
	out[2] = in[2] / len;
	out[3] = in[3] / len;
}

extern void vec4_print(vec4_t in)
{
	printf("%.2f/%.2f/%.2f/%.2f", in[0], in[1], in[2], in[3]);
}
