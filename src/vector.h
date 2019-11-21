#ifndef VASALL_VECTOR_H
#define VASALL_VECTOR_H

#define M_PI 3.14159265358979323846

typedef struct vsRect {
	float x;
	float y;
	float w;
	float h;
} vsRect;

/*
 * A simple 2d-Vector struct used the
 * same as COORD, but with 32-bit numbers
 * instead of 16.
 */
typedef struct Vec2 {
	float x;
	float y;
} Vec2;

/* ==== DEFINE PROTOTYPES ==== */
void vec_add(Vec2 *dst, Vec2 *val);
Vec2 vec_add_ret(Vec2 *vec1, Vec2 *vec2);
void vec_sub(Vec2 *dst, Vec2 *val);
Vec2 vec_sub_ret(Vec2 *vec1, Vec2 *vec2);
void vec_scl(Vec2 *dst, float fac);
Vec2 vec_scl_ret(Vec2 *vec, float fac);
void vec_set(Vec2 *dst, float x, float y);
void vec_cpy(Vec2 *dst, Vec2 *src);
float vec_mag(Vec2 *vec);
float vec_magsq(Vec2 *vec);
void vec_lim(Vec2 *vec, float lim);
Vec2 vec_lim_ret(Vec2 *vec, float lim);
float vec_dot(Vec2 *vec1, Vec2 *vec2);
void vec_nrm(Vec2 *vec);
Vec2 vec_nrm_ret(Vec2 *vec);
void vec_matmul(Vec2 *vec, float m11, float m21, float m12, float m22);
Vec2 vec_matmul_ret(Vec2 *vec, float m11, float m21, float m12, float m22);
Vec2 vec_rot_ret(Vec2 *vec, float rad);
void vec_rot(Vec2 *vec, float rad);
Vec2 vec_rot_deg_ret(Vec2 *vec, int deg);
void vec_rot_deg(Vec2 *vec, int deg);

#endif
