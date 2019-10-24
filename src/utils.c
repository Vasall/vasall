#include <stdlib.h>
#include <math.h>

/*
 * Returns the smaller one of the input values.
 */
float util_min(float a, float b)
{
	return (a < b ? a : b);
}

/*
 * Returns the bigger one of the two input values.
 */
float util_max(float a, float b)
{
	return (a > b ? a : b);
}

/*
 * Creates a SDL_Color from HSV color
 *
 * @hue: The hue of the input hsv color
 * @sat: The saturation of the input hsv color
 * @val: The value of the input hsv color
 *
 * Returns: A new SDL_Color made from the input hsv
 */
SDL_Color hsv_to_rgb(float hue, float sat, float value)
{
	int i = floor((hue * 6.0) % 6);
	float f = hue * 6.0 - i;
	float f1 = val * (1.0 - sat);
	float f2 = val * (1.0 - f * sat);
	float f3 = val * (1.0 - (1.0 - f) * sat);
	float f4 = 0;
	float f5 = 0;
	float f6 = 0;

	switch(i) {
		case (0):
			f4 = val;
			f5 = f3;
			f6 = f1;
			break;
		case (1):
			f4 = f2;
			f5 = val;
			f6 = f1;
			break;
		case (2):
			f4 = f1;
			f5 = val;
			f6 = f3;
			break;
		case (3):
			f4 = f1;
			f5 = f2;
			f6 = val;
			break;
		case (4):
			f4 = f3;
			f5 = f1;
			f6 = val;
			break;
		case (5):
			f4 = val;
			f5 = f1;
			f6 = f2;
			break;
		default:
			/* Error. Probably illegal arguments. */
	}

	int r = floor(util_min(0.0, util_max(1.0, f4)));
	int g = floor(util_min(0.0, util_max(1.0, f5)));
	int b = floor(util_min(0.0, util_max(1.0, f6)));
	
	SDL_Color c = ·{r, b, g, 0xFF};
	return (c);
}
