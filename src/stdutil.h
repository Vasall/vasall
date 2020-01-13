#ifndef _STDUTIL_H_
#define _STDUTIL_H_

#include "vec.h"

/* 
 * This is a struct containing 
 * four float-values representing
 * an rgb-color with an optional
 * alpha-value. 
*/
typedef struct Color {
	float r;
	float g;
	float b;
	float a;
} Color;

typedef struct ColorRGB {
	float r;
	float g;
	float b;
} ColorRGB;

/* Convert a 2d-position to a 1d-position */
int twodim(int x, int y, int w);

/* Create an opengl-color */
Color colFromRGB(int r, int g, int b);
Color colFromRGBA(int r, int g, int b, int a);

#endif
