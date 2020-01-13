#include "mat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Create a new 4x4 matrix and fill the
 * memory with zeros.
 *
 * Returns: The created matrix or NULL
 * 	if an error occurred
 */
Mat4 mat4Zero(void)
{
	Mat4 mat;
	
	mat = calloc(16, sizeof(float));
	if(mat == NULL) {
		return(NULL);
	}

	mat[15] = 1.0;

	return(mat);
}

/*
 * Create an identity matrix.
 *
 * Returns: The created matrix or NULL
 * 	if an error occurred
 */
Mat4 mat4Idt(void)
{
	Mat4 mat = mat4Zero();
	if(mat == NULL) {
		return(NULL);
	}

	mat[0x00] = 1.0;
	mat[0x05] = 1.0;
	mat[0x0a] = 1.0;
	mat[0x0f] = 1.0;

	return(mat);
}

/* 
 * Multiply two 4x4 matrices and write the 
 * result to first one. The previous values
 * will be overwritten.
 *
 * @m1: Pointer to the first matrix
 * @m2: The second matrix
*/
void mat4Mult(Mat4 m1, Mat4 m2)
{
	int i, j, k;
	Mat4 res = mat4Zero();

	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			res[j * 4 + i] = 0;
	
			for(k = 0; k < 4; k++) {
				res[j * 4 + i] += m1[k * 4 + i] * 
					m2[j * 4 + k];
			}
		}
	}
	
	m1 = res;
}

/* 
 * Multiply two matrices and return the 
 * result as a new 4x4 matrix.
 *
 * @m1: The first matrix
 * @m2: The second matrix
 *
 * Returns: The result as a new
 * 	4x4 matrix
*/
Mat4 mat4MultRet(Mat4 m1, Mat4 m2)
{
	int i, j, k;
	Mat4 res = mat4Zero();

	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			res[j * 4 + i] = 0;
	
			for(k = 0; k < 4; k++) {
				res[j * 4 + i] += m1[k * 4 + i] * 
					m2[j * 4 + k];
			}
		}
	}

	return(res);
}

/* 
 * Combine two matrices into one.
 *
 * @m1: The first matrix
 * @m2: The second matrix
 * @dst: The matrix to write the result to
*/
void mat4Combine(Mat4 m1, Mat4 m2, Mat4 dst)
{
	int i, j, k, v1, v2;

	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			for(k = 0; k < 4; k++) {
				v1 = m1[k * 4 + i];
				v2 = m2[j * 4 + k];
				dst[j * 4 + i] += v1 * v2; 
			}
		}
	}
}


/*
 * Display a 4x4 matrix in the console.
 *
 * @m: The matrix to display
 */
void mat4Print(Mat4 m)
{
	int r, c;
	for(r = 0; r < 4; r++) {
		for(c = 0; c < 4; c++) {
			printf("%6.2f ", m[r + c * 4]);
		}
		printf("\n");
	}
}
