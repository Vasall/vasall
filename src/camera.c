#define NOT_DEFINE_CAMERA
#include "global.h"

/*
 * Convert a relative position in units to an 
 * absolute position in the window in pixels.
 *
 * @cam: The camera to use to convert the position
 * @x: The relative x-position in units
 * @y: The relative y-position in units
 * @xp: Pointer to variable to write x-position to
 * @yp: Pointer to variable to write y-position to
*/
extern void cam_getpos(vsCamera *cam, float x, float y, int *xp, int *yp)
{
	Vec2 *pos = &cam->pos;
	int f = cam->zoom * cam->tilesz;
	*xp = (g_context->win_w / 2) - (pos->x * f) + (x * f);
	*yp = (g_context->win_h / 2) - (pos->y * f) + (y * f);
}

/*
 * Convert a relative size in units to an
 * absolute size in the window in pixels.
 *
 * @cam: The camera to use to convert the size
 * @x: The width in units to convert
 * @y: The height in units to convert
 * @xp: Pointer to variable to write width to
 * @yp: Pointer to variable to write height to
*/
extern void cam_convun(vsCamera *cam, float x, float y, int *xp, int *yp)
{
	int f = cam->zoom * cam->tilesz;
	*xp = x * f;
	*yp = y * f;
}

