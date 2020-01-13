#ifndef _MAT_H_
#define _MAT_H_

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


#endif
