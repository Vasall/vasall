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

/* ==== DEFINE PROTOTYPES ==== */
char inRect(SDL_Rect *rect, Vec2 *pos);

#endif
