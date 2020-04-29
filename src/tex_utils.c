#include "tex_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"


V_API int tex_load_png(char *pth, uint8_t **buf, int *w, int *h)
{
	int width, height, size;
	SDL_Surface *surf;
	uint8_t *buffer;

	if(!(surf = IMG_Load(pth)))
		return -1;

	width = surf->w;
	height = surf->h;
	size = surf->pitch * surf->h;

	if(!(buffer = malloc(size)))
		return -1;

	memcpy(buffer, surf->pixels, size);

	*buf = buffer;
	*w = width;
	*h = height;
	return 0;
}
