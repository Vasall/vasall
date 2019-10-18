#include <math.h>
#include <SDL.h>
#include "xmath.h"

/*
 * Check if a specified position is inside of a rectangle.
 * 
 * @rect: The rectangle to check if the point is in
 * @pos: The position to check if is in the rect
 *
 * Return: Either 1 if the point is in the rect, or 0 if not
*/
char inRect(SDL_Rect *rect, Vec2 *pos){
	if(pos->x >= rect->x && pos->x <= rect->x + rect->w && 
			pos->y >= rect->y && pos->y <= rect->y + rect->h) {
		return (1);
	}
	return (0);
}
