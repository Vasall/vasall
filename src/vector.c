#include <math.h>
#include <SDL.h>
#include "vector.h"

/*
 * Check, whether a specified position is inside of a rectangle. If the
 * point is just on the border, it will still be considered inside of the
 * rectangle.
 * 
 * @rect: The rectangle
 * @pos: The position
 *
 * Returns: 1 if position is inside of rectangle, 0 if it is outside
 */
char in_rect(SDL_Rect *rect, Vec2 *pos) 
{
	if(pos->x >= rect->x && pos->x <= rect->x + rect->w && 
			pos->y >= rect->y && pos->y <= rect->y + rect->h) {
		return (1);
	}
	return (0);
}

/**
 * Adds two vectors together.
 *
 * @dst: The destination vector. val will be add to this vector
 * @val: The vector to add to dst
 */
void add_vecs(Vec2 *dst, Vec2 *val) 
{
	/* dst.x += val.x; */
	/* dst.y += val.y; */
}
