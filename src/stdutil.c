#include <stdlib.h>

#include "stdutil.h"

/*
 * Convert a 2d-position to a 1d-position
 * in a single-dimension-array.
 *
 * @x: The x-position in the array
 * @y: The y-position in the array
 * @w: The width of the array
 */
int twodim(int x, int y, int w)
{
	return(x + (y * w));
}

/*
 * Create a new color-struct and set
 * the rgb-values. The given values will
 * be divided by 255, as opengl wants
 * floating-point-values between 0 and 1.
 * The transparency will be set to 1.
 *
 * @r: The red-value ranging from 0-255
 * @g: The green-value ranging from 0-255
 * @b: The blue-value ranging from 0-255
 *
 * Returns: A new color-struct containing
 * 	the specified values as floats
 */
Color colFromRGB(int r, int g, int b)
{
	Color col;
	col.r = r / 255;
	col.g = g / 255;
	col.b = b / 255;
	col.a = 1.0;
	return(col);
}

/*
 * Create a new color-struct and set
 * the rgba-values. The given values will
 * be divided by 255, as opengl wants
 * floating-point-values between 0 and 1.
 *
 * @r: The red-value ranging from 0-255
 * @g: The green-value ranging from 0-255
 * @b: The blue-value ranging from 0-255
 * @a: The tranparency ranging from 0-255
 *
 * Returns: A new color-struct containing
 * 	the specified values as floats
 */
Color colFromRGBA(int r, int g, int b, int a)
{
	Color col;
	col.r = r / 255;
	col.g = g / 255;
	col.b = b / 255;
	col.a = a / 255;
	return(col);	
}

