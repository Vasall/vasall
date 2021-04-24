#include "extmath.h"

#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern float clamp(float v)
{
	if(v < 0.0) return 0.0;
	if(v > 1.0) return 1.0;
	return v;
}


extern float dist(float x, float y, float xp, float yp)
{
	return (sqrt((x - xp) * (x - xp) + (y - yp) * (y - yp)));
}


extern float round(float v)
{
	v *= 10.0f;
	v = (v > (floor(v) + 0.5f)) ? ceil(v) : floor(v);
	v /= 10.0f;

	return v;
}

static float lineSlope(vec2_t a)
{ 
	if(a[0] == 0)
		return 0;

	return a[1] / a[0]; 
}

extern int line_cross(vec2_t p0, vec2_t v0, vec2_t p1, vec2_t v1, vec2_t out)
{
	float slope_a = lineSlope(v0);
	float slope_b = lineSlope(v1);
	float del = slope_a - slope_b;

	if(slope_a == slope_b)
		return 0;

	out[0] = (slope_a * p0[0] - slope_b * p1[0] + p1[1] - p0[1]) / del;
	out[1] = slope_b * (out[0] - p1[0]) + p1[1];
 
	return 1;
}
