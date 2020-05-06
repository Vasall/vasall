#include "mat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void mat3_zero(mat3_t mat) 
{
	memset(mat, 0, MAT3_SIZE);
}

extern void mat3_idt(mat3_t mat) 
{
	mat3_zero(mat);
	mat[0x0] = 1.0;
	mat[0x4] = 1.0;
	mat[0x8] = 1.0;
}

extern void mat3_cpy(mat3_t dst, mat3_t src) 
{
	memcpy(dst, src, MAT3_SIZE);
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

extern void mat3_print(mat3_t mat)
{
	int r, c;

	for(r = 0; r < 3; r++) {
		for(c = 0; c < 3; c++)
			printf("%6.2f ", mat[c * 3 + r]);

		printf("\n");
	}
}

extern void mat4_zero(mat4_t mat)
{
	memset(mat, 0, MAT4_SIZE);
	mat[0xf] = 1.0;
}

extern void mat4_idt(mat4_t mat)
{
	mat4_zero(mat);
	mat[0x00] = 1.0;
	mat[0x05] = 1.0;
	mat[0x0a] = 1.0;
	mat[0x0f] = 1.0;
}

extern void mat4_cpy(mat4_t dst, mat4_t src)
{
	memcpy(dst, src, MAT4_SIZE);
}

extern void mat4_mult(mat4_t m1, mat4_t m2, mat4_t res)
{
	int i, j, k;
	mat4_zero(res);

	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			res[j * 4 + i] = 0;
	
			for(k = 0; k < 4; k++) {
				res[j * 4 + i] += m1[k * 4 + i] * 
					m2[j * 4 + k];
			}
		}
	}
}

extern void mat4_print(mat4_t mat)
{
	int r, c;
	for(r = 0; r < 4; r++) {
		for(c = 0; c < 4; c++) {
			printf("%6.2f ", mat[r + c * 4]);
		}
		printf("\n");
	}
}
