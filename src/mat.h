#ifndef _MAT_H_
#define _MAT_H_

typedef float *Mat4;

/* Create a new 4x4 matrix filled with zeros */
Mat4 mat4Zero(void);

/* Create an identity 4x4 matrix */
Mat4 mat4Idt(void);

/* Multiply two matrices and write result to first one */
void mat4Mult(Mat4 m1, Mat4 m2);

/* Multiply two matrices and return the result */
Mat4 mat4MultRet(Mat4 m1, Mat4 m2);

/* Combine two matrices into one */
void mat4Combine(Mat4 m1, Mat4 m2, Mat4 dst);

/* Display a 4x4 matrix in the console */
void mat4Print(Mat4 m);


#endif
