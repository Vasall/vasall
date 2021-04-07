#include "matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern void mat3_zero(mat3_t m) 
{
	memset(m, 0, MAT3_SIZE);
}

extern void mat3_idt(mat3_t m) 
{
	mat3_zero(m);
	m[0x0] = 1.0;
	m[0x4] = 1.0;
	m[0x8] = 1.0;
}

extern void mat3_cpy(mat3_t out, mat3_t in) 
{
	memcpy(out, in, MAT3_SIZE);
}

extern void mat3_mult(mat3_t m1, mat3_t m2, mat3_t res)
{
	int i, j, k;
	mat4_t ret;
	mat4_zero(ret);
	mat4_zero(res);

	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			ret[j * 4 + i] = 0;

			for(k = 0; k < 3; k++)
				ret[j * 3 + i] += m1[k * 3 + i] * 
					m2[j * 3 + k];
		}
	}

	mat4_cpy(res, ret);
}

extern void mat3_print(mat3_t m)
{
	int r, c;

	for(r = 0; r < 3; r++) {
		for(c = 0; c < 3; c++)
			printf("%6.2f ", m[c * 3 + r]);

		printf("\n");
	}
}

extern void mat4_zero(mat4_t m)
{
	int i;

	for(i = 0; i < 16; i++)
		m[i] = 0.0;
}

extern void mat4_idt(mat4_t m)
{
	mat4_zero(m);
	m[0x00] = 1.0;
	m[0x05] = 1.0;
	m[0x0a] = 1.0;
	m[0x0f] = 1.0;
}

extern void mat4_cpy(mat4_t out, mat4_t in)
{
	if(in)
		memcpy(out, in, MAT4_SIZE);
}

extern void mat4_mult(mat4_t m1, mat4_t m2, mat4_t out)
{
	int i, j, k;
	float a, b;
	mat4_t conv;
	mat4_zero(conv);

	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			for(k = 0; k < 4; k++) {
				a = m1[k * 4 + i];
				b = m2[j * 4 + k];

				conv[j * 4 + i] += a * b;
			}
		}
	}

	mat4_cpy(out, conv);
}

extern void mat4_inv(mat4_t out, mat4_t in)
{
	double inv[16], det;
	int i;

	inv[0] = in[5]  * in[10] * in[15] - 
		in[5]  * in[11] * in[14] - 
		in[9]  * in[6]  * in[15] + 
		in[9]  * in[7]  * in[14] +
		in[13] * in[6]  * in[11] - 
		in[13] * in[7]  * in[10];

	inv[4] = -in[4]  * in[10] * in[15] + 
		in[4]  * in[11] * in[14] + 
		in[8]  * in[6]  * in[15] - 
		in[8]  * in[7]  * in[14] - 
		in[12] * in[6]  * in[11] + 
		in[12] * in[7]  * in[10];

	inv[8] = in[4]  * in[9] * in[15] - 
		in[4]  * in[11] * in[13] - 
		in[8]  * in[5] * in[15] + 
		in[8]  * in[7] * in[13] + 
		in[12] * in[5] * in[11] - 
		in[12] * in[7] * in[9];

	inv[12] = -in[4]  * in[9] * in[14] + 
		in[4]  * in[10] * in[13] +
		in[8]  * in[5] * in[14] - 
		in[8]  * in[6] * in[13] - 
		in[12] * in[5] * in[10] + 
		in[12] * in[6] * in[9];

	inv[1] = -in[1]  * in[10] * in[15] + 
		in[1]  * in[11] * in[14] + 
		in[9]  * in[2] * in[15] - 
		in[9]  * in[3] * in[14] - 
		in[13] * in[2] * in[11] + 
		in[13] * in[3] * in[10];

	inv[5] = in[0]  * in[10] * in[15] - 
		in[0]  * in[11] * in[14] - 
		in[8]  * in[2] * in[15] + 
		in[8]  * in[3] * in[14] + 
		in[12] * in[2] * in[11] - 
		in[12] * in[3] * in[10];

	inv[9] = -in[0]  * in[9] * in[15] + 
		in[0]  * in[11] * in[13] + 
		in[8]  * in[1] * in[15] - 
		in[8]  * in[3] * in[13] - 
		in[12] * in[1] * in[11] + 
		in[12] * in[3] * in[9];

	inv[13] = in[0]  * in[9] * in[14] - 
		in[0]  * in[10] * in[13] - 
		in[8]  * in[1] * in[14] + 
		in[8]  * in[2] * in[13] + 
		in[12] * in[1] * in[10] - 
		in[12] * in[2] * in[9];

	inv[2] = in[1]  * in[6] * in[15] - 
		in[1]  * in[7] * in[14] - 
		in[5]  * in[2] * in[15] + 
		in[5]  * in[3] * in[14] + 
		in[13] * in[2] * in[7] - 
		in[13] * in[3] * in[6];

	inv[6] = -in[0]  * in[6] * in[15] + 
		in[0]  * in[7] * in[14] + 
		in[4]  * in[2] * in[15] - 
		in[4]  * in[3] * in[14] - 
		in[12] * in[2] * in[7] + 
		in[12] * in[3] * in[6];

	inv[10] = in[0]  * in[5] * in[15] - 
		in[0]  * in[7] * in[13] - 
		in[4]  * in[1] * in[15] + 
		in[4]  * in[3] * in[13] + 
		in[12] * in[1] * in[7] - 
		in[12] * in[3] * in[5];

	inv[14] = -in[0]  * in[5] * in[14] + 
		in[0]  * in[6] * in[13] + 
		in[4]  * in[1] * in[14] - 
		in[4]  * in[2] * in[13] - 
		in[12] * in[1] * in[6] + 
		in[12] * in[2] * in[5];

	inv[3] = -in[1] * in[6] * in[11] + 
		in[1] * in[7] * in[10] + 
		in[5] * in[2] * in[11] - 
		in[5] * in[3] * in[10] - 
		in[9] * in[2] * in[7] + 
		in[9] * in[3] * in[6];

	inv[7] = in[0] * in[6] * in[11] - 
		in[0] * in[7] * in[10] - 
		in[4] * in[2] * in[11] + 
		in[4] * in[3] * in[10] + 
		in[8] * in[2] * in[7] - 
		in[8] * in[3] * in[6];

	inv[11] = -in[0] * in[5] * in[11] + 
		in[0] * in[7] * in[9] + 
		in[4] * in[1] * in[11] - 
		in[4] * in[3] * in[9] - 
		in[8] * in[1] * in[7] + 
		in[8] * in[3] * in[5];

	inv[15] = in[0] * in[5] * in[10] - 
		in[0] * in[6] * in[9] - 
		in[4] * in[1] * in[10] + 
		in[4] * in[2] * in[9] + 
		in[8] * in[1] * in[6] - 
		in[8] * in[2] * in[5];

	det = in[0] * inv[0] + in[1] * inv[4] + in[2] * inv[8] + in[3] * inv[12];

	if(det == 0)
		return;

	det = 1.0 / det;

	for(i = 0; i < 16; i++)
		out[i] = (float)(inv[i] * det);
}

extern void mat4_transp(mat4_t out, mat4_t in)
{
	mat4_t tmp;

	tmp[0x0] = in[0x0];
	tmp[0x1] = in[0x4];
	tmp[0x2] = in[0x8];
	tmp[0x3] = in[0xc];
	tmp[0x4] = in[0x1];
	tmp[0x5] = in[0x5];
	tmp[0x6] = in[0x9];
	tmp[0x7] = in[0xd];
	tmp[0x8] = in[0x2];
	tmp[0x9] = in[0x6];
	tmp[0xa] = in[0xa];
	tmp[0xb] = in[0xe];
	tmp[0xc] = in[0x3];
	tmp[0xd] = in[0x7];
	tmp[0xe] = in[0xb];
	tmp[0xf] = in[0xf];

	mat4_cpy(out, tmp);
}

extern void mat4_print(mat4_t m)
{
	int r, c;
	for(r = 0; r < 4; r++) {
		for(c = 0; c < 4; c++)
			printf("%.6f ", m[r + c * 4]);

		printf("\n");
	}
}

extern void mat4_std(mat4_t m)
{
	int i;

	for(i = 0; i < 16; i++) {
		if(m[i] == 0.0) {
			m[i] = 0.0;
		}
	}
}

extern void mat4_pfpos(mat4_t m, vec3_t v)
{	
	m[0xc] = v[0];
	m[0xd] = v[1];
	m[0xe] = v[2];

	m[0xf] = 1.0;
}

extern void mat4_rfqat(mat4_t m, vec4_t v)
{
	m[0x0] = 1.0 - 2.0 * (v[2] * v[2] + v[3] * v[3]);
	m[0x1] = 2.0 * (v[1] * v[2]) + 2.0 * (v[3] * v[0]);
	m[0x2] = 2.0 * (v[1] * v[3]) - 2.0 * (v[2] * v[0]);
	m[0x3] = 0.0;

	m[0x4] = 2.0 * (v[1] * v[2]) - 2.0 * (v[3] * v[0]);
	m[0x5] = 1.0 - 2.0 * (v[0] * v[0]) - 2.0 * (v[3] * v[3]);
	m[0x6] = 2.0 * (v[2] * v[3]) + 2.0 * (v[1] * v[0]);
	m[0x7] = 0.0;

	m[0x8] = 2.0 * (v[1] * v[3]) + 2.0 * (v[2] * v[0]);
	m[0x9] = 2.0 * (v[2] * v[3]) - 2.0 * (v[1] * v[0]);
	m[0xa] = 1.0 - 2.0 * (v[1] * v[1]) - 2.0 * (v[2] * v[2]);
	m[0xb] = 0.0;

	m[0xf] = 1.0;
}

extern void mat4_rfvec(mat4_t m, vec3_t v)
{
	vec3_t up = {0, 0, 1};
	vec3_t xaxis;
	vec3_t yaxis;

	vec3_cross(up, v, xaxis);
	vec3_nrm(xaxis, xaxis);

	vec3_cross(v, xaxis, yaxis);
	vec3_nrm(yaxis, yaxis);

	mat4_idt(m);

	m[0x0] = xaxis[0];
	m[0x1] = yaxis[0];
	m[0x2] = v[0];

	m[0x4] = xaxis[1];
	m[0x5] = yaxis[1];
	m[0x6] = v[1];

	m[0x8] = xaxis[2];
	m[0x9] = yaxis[2];
	m[0xa] = v[2];
}

/* x: pitch, y: roll, z: yaw; angles in euler */
extern void mat4_rfagl(mat4_t m, vec3_t v)
{
	float x = DEG_TO_RAD(v[0]);
	float y = DEG_TO_RAD(v[1]);
	float z = DEG_TO_RAD(v[2]);

	/* Roll */
	float cosY = cos(y);
	float sinY = sin(y);

	/* Pitch */
	float cosP = cos(x);
	float sinP = sin(x);

	/* Yaw */
	float cosR = cos(z);
	float sinR = sin(z);

	mat4_idt(m);
	m[0x0] = cosY * cosR + sinY * sinP * sinR;
	m[0x1] = cosR * sinY * sinP - sinR * cosY;
	m[0x2] = cosP * sinY;

	m[0x4] = cosP * sinR;
	m[0x5] = cosR * cosP;
	m[0x6] = -sinP;

	m[0x8] = sinR * cosY * sinP - sinY * cosR;
	m[0x9] = sinY * sinR + cosR * cosY * sinP;
	m[0xa] = cosP * cosY;


}

extern void mat4_pfpos_s(mat4_t m, float x, float y, float z)
{	
	m[0xc] = x;
	m[0xd] = y;
	m[0xe] = z;

	m[0xf] = 1.0;
}

extern void mat4_rfqat_s(mat4_t m, float w, float x, float y, float z)
{
	m[0x0] = 1.0 - 2.0 * (y * y + z * z);
	m[0x1] = 2.0 * (x * y) + 2.0 * (z * w);
	m[0x2] = 2.0 * (x * z) - 2.0 * (y * w);
	m[0x3] = 0.0;

	m[0x4] = 2.0 * (x * y) - 2.0 * (z * w);
	m[0x5] = 1.0 - 2.0 * (x * x) - 2.0 * (z * z);
	m[0x6] = 2.0 * (y * z) + 2.0 * (x * w);
	m[0x7] = 0.0;

	m[0x8] = 2.0 * (x * z) + 2.0 * (y * w);
	m[0x9] = 2.0 * (y * z) - 2.0 * (x * w);
	m[0xa] = 1.0 - 2.0 * (x * x) - 2.0 * (y * y);
	m[0xb] = 0.0;

	m[0xf] = 1.0;
}

/* see mat4_rfagl() for parameter info */
extern void mat4_rfagl_s(mat4_t m, float x, float y, float z)
{
	vec3_t v;
	mat4_t c;

	vec3_set(v, x, y, z);
	mat4_rfagl(c, v);
	mat4_cpy(m, c);
}
