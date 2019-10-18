#include <math.h>
#include <SDL.h>
#include "xmath.h"

/*
 * Check, whether a specified position is inside of a rectangle.
 * 
 * @param rect: The rectanlge
 * @param pos: The position
 *
 * Returns: 1 if position is inside of rectangle, 0 if it is outside.
 * 			Points on border are considered to be inside.
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
 * @param dst: The destination vector. val will be add to this vector
 * @param val: The vector to add to dst
 */
void add_vecs(Vec2 *dst, Vec2 *val) 
{
	//dst.x += val.x;
	//dst.y += val.y;
}
