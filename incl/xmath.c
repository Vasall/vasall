#include <math.h>
#include <SDL.h>
#include "xmath.h"

/*
<<<<<<< HEAD
 * Check, whether a specified position is inside of a rectangle.
 * 
 * @param rect: The rectanlge
 * @param pos: The position
 *
 * Returns: 1 if position is inside of rectangle, 0 if it is outside.
 * 			Points on border are considered to be inside.
 */
char in_rect(SDL_Rect *rect, Vec2 *pos) {
=======
 * Check if a specified position is inside of a rectangle.
 * 
 * @rect: The rectangle to check if the point is in
 * @pos: The position to check if is in the rect
 *
 * Return: Either 1 if the point is in the rect, or 0 if not
*/
char inRect(SDL_Rect *rect, Vec2 *pos){
>>>>>>> 6106ffb2df3e5cb70069372742b569ba5267e90f
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
void add_vecs(Vec2 *dst, Vec2 *val) {
	//dst.x += val.x;
	//dst.y += val.y;
}
