#ifndef _MAT_H
#define _MAT_H

#define MAT3_SIZE (sizeof(float)*9)
typedef float mat3_t[9];

void mat3_zero(mat3_t mat);
void mat3_idt(mat3_t mat);
void mat3_cpy(mat3_t dst, mat3_t src);
void mat3_mult(mat3_t m1, mat3_t m2, mat3_t res);
void mat3_print(mat3_t mat);

#define MAT4_SIZE (sizeof(float)*16)
typedef float mat4_t[16];

void mat4_zero(mat4_t mat);
void mat4_idt(mat4_t mat);
void mat4_cpy(mat4_t dst, mat4_t src);
void mat4_mult(mat4_t m1, mat4_t m2, mat4_t res);
void mat4_print(mat4_t mat);

#endif
