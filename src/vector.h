#ifndef _VECTOR_H_
#define _VECTOR_H_

/*
 * A simple 2d-Vector struct used the
 * same as COORD, but with 32-bit numbers
 * instead of 16.
 */
typedef struct Vec2 {
	int x;
	int y;
} Vec2;

/* ==== DEFINE PROTOTYPES ==== */
char in_rect(SDL_Rect *rect, Vec2 *pos);
void add_vecs(Vec2 *dst, Vec2 *val);

#endif
