#ifndef _MAT_H_
#define _MAT_H_

#define MAT3_SIZE (sizeof(float)*9)
typedef float Mat3[9];

#define MAT4_SIZE (sizeof(float)*16)
typedef float Mat4[16];

/* Create a new 4x4 matrix filled with zeros */
void mat4Zero(Mat4 mat);

/* Create an identity 4x4 matrix */
void mat4Idt(Mat4 mat);

/* Copy the values from one matrix to another */
void mat4Cpy(Mat4 dst, Mat4 src);

/* Multiply two matrices */
void mat4Mult(Mat4 m1, Mat4 m2, Mat4 res);

/* Display a 4x4 matrix in the console */
void mat4Print(Mat4 m);

/* Create a new 3x3 Matrix filled with zeros */
void mat3Zero(Mat3 mat);

/* Create an identity 3x3 matrix */
void mat3Idt(Mat3 mat);

/* Copy the values from one 3x3 matrix to another */
void mat3Cpy(Mat3 dst, Mat3 src);

/* Multiply two 3x3 matricies */
void mat3Mult(Mat3 m1, Mat3 m2, Mat3 res);

/* Print a 3x3 matrix to the console */
void mat3Print(Mat3 mat);


#endif
