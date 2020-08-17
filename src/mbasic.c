#include "mbasic.h"

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
