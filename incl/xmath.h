#ifndef _XMATH_H_
#define _XMATH_H_

/*
 * A simple 2d-Vector struct used the
 * same as CORD, but with 32-bit numbers
 * instead of 16.
*/
typedef struct Vec2 {
	int x;
	int y;
} Vec2;

<<<<<<< HEAD
// ==== DEFINE PROTOTYPES ====
char in_rect(SDL_Rect *rect, Vec2 *pos);
void add_vecs(Vec2 *dst, Vec2 *val);
=======
/* ==== DEFINE PROTOTYPES ==== */
char inRect(SDL_Rect *rect, Vec2 *pos);
>>>>>>> 6106ffb2df3e5cb70069372742b569ba5267e90f

#endif
