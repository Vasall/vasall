#ifndef _MATRIX_H
#define _MATRIX_H

#define NULLMAT ((float *)NULL)

#define MAT3_SIZE (sizeof(float)*9)
typedef float mat3_t[9];

extern void mat3_zero(mat3_t mat);
extern void mat3_idt(mat3_t mat);
extern void mat3_cpy(mat3_t dst, mat3_t src);
extern void mat3_mult(mat3_t m1, mat3_t m2, mat3_t res);
extern void mat3_print(mat3_t mat);


#define MAT4_SIZE (sizeof(float)*16)
typedef float mat4_t[16];

extern void mat4_zero(mat4_t m);
extern void mat4_idt(mat4_t m);
extern void mat4_cpy(mat4_t out, mat4_t in);
extern void mat4_mult(mat4_t m1, mat4_t m2, mat4_t out);
extern void mat4_inv(mat4_t out, mat4_t in);
extern void mat4_transp(mat4_t out, mat4_t in);
extern void mat4_print(mat4_t m);

extern void mat4_std(mat4_t m);

extern void mat4_fpos(mat4_t m, float x, float y, float z);
extern void mat4_fqat(mat4_t m, float w, float x, float y, float z);

#endif
