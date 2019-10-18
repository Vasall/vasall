#include <math.h>
#include <SDL.h>
#include "xmath.h"

/*
 * Check if a specified position is inside of a rectangle.
 * The function returns 1 if the position is inside of the 
 * rectangle, and 0 if not.
 * 
*/
char inRect(SDL_Rect *rect, Vec2 *pos){
	if(pos->x >= rect->x && pos->x <= rect->x + rect->w && 
			pos->y >= rect->y && pos->y <= rect->y + rect->h) {
		return (1);
	}
	return (0);
}
