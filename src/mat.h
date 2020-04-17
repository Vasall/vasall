#ifndef _MAT_H
#define _MAT_H

#define MAT3_SIZE (sizeof(float)*9)
typedef float mat3_t[9];

/*
 * Initialite a 3x3 matrix and set all values to 0,
 * expect the last one, which will be set to 1.
 *
 * @mat: The matrix to initialize
 */
void mat3_zero(mat3_t mat);

/*
 * Initialize a 3x3-identity-matrix.
 *
 * @mat: The matrix to initialize
 */
void mat3_idt(mat3_t mat);

/*
 * Copy the values of one 3x3-matrix to another.
 *
 * @dst: The matrix to copy the values to
 * @src: The matrix to copy the values from
 */
void mat3_cpy(mat3_t dst, mat3_t src);

/*
 * Multiply two 3x3-matrices with one another, then return the resulting 
 * matrix. The result-matrix can be the same as one of the input-matrices.
 *
 * @m1: The first matrix
 * @m2: The second matrix
 * @res: The matrix to write the result to
 */
void mat3_mult(mat3_t m1, mat3_t m2, mat3_t res);

/*
 * Display a 3x3-matrix in the console.
 *
 * @mat: The matrix to display
 */
void mat3_print(mat3_t mat);

#define MAT4_SIZE (sizeof(float)*16)
typedef float mat4_t[16];

void mat4_zero(mat4_t mat);
void mat4_idt(mat4_t mat);
void mat4_cpy(mat4_t dst, mat4_t src);
void mat4_mult(mat4_t m1, mat4_t m2, mat4_t res);
void mat4_print(mat4_t mat);

#endif
