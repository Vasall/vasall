#include "mat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Create a new 4x4 matrix and fill the
 * memory with zeros.
 *
 * @mat: The matrix to initialize
 */
void mat4Zero(Mat4 mat)
{
	memset(mat, 0, MAT4_SIZE);
	mat[0xf] = 1.0;
}

/*
 * Create an identity matrix.
 *
 * @mat: The matrix to initialize
 */
void mat4Idt(Mat4 mat)
{
	mat4Zero(mat);
	mat[0x00] = 1.0;
	mat[0x05] = 1.0;
	mat[0x0a] = 1.0;
	mat[0x0f] = 1.0;
}


/*
 * Copy the values from one matrix to another.
 * 
 * @dst: The matrix to write to
 * @src: The matrix to read from
*/
void mat4Cpy(Mat4 dst, Mat4 src)
{
	memcpy(dst, src, MAT4_SIZE);
}

/* 
 * Multiply two 4x4 matrices and write the 
 * result to result-matrix. The previous values
 * will be overwritten.
 *
 * @m1: Pointer to the first matrix
 * @m2: The second matrix
 * @res: The matrix to write the result to
*/
void mat4Mult(Mat4 m1, Mat4 m2, Mat4 res)
{
	int i, j, k;
	mat4Zero(res);

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

/*
 * Fills the specified matrix with zeros
 *
 * @mat: The matrix to be filled with zeros
 */
void mat3Zero(Mat3 mat) {
	memset(Mat3, 0, MAT3_SIZE);
}

/*
 * Creates an Identity matrix
 *
 * @mat: The matrix to edit
 */
void mat3Idt(Mat3 mat) {
	mat3Zero(mat);
	mat[0x0] = 1.0;
	mat[0x4] = 1.0;
	mat[0x9] = 1.0;
}

/*
 * Copies a matrix
 *
 * @dst: The destination matrix to copy to
 * @src: The source matrix to copy from
 */
void mat3Cpy(Mat3 dst, Mat3 src) {
	memcpy(src, dst, MAT3_SIZE);
}

/*
 * Multiplies 2 3x3 matricies together and writes
 * the result into res
 *
 * @m1: The first matrix factor
 * @m2: The second matrix factor
 * @res: The resulting matrix
 */
void mat3Mult(Mat3 m1, Mat3 m2, Mat3 res) {
	int i, j, k;
	mat4Zero(res);

	/* FIXME when m1 is the same as res, the calculation will fail */

	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			res[j * 4 + i] = 0;

			for(k = 0; k < 3; k++) {
				res[j * 3 + i] += m1[k * 3 + i] * m2[j * 3 + k];
			}
		}
	}
}

/*
 * Prints a 3x3 matrix to the console
 *
 * @mat: The 3x3 matrix
 */
void mat3Print(Mat3 mat) {
	int r, c;

	for(r = 0; r < 3; r++) {
		for(c = 0; c < 3; c++) {
			printf("%6.2f ", mat[c * 3 + r]);
		}
		printf("\n");
	}
}
