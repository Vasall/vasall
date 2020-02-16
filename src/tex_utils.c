#include "tex_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

/*
 * Load a png-file and write the pixel-data into
 * the given buffer. The set both the width and
 * the height of the texture. Note that the buffer
 * will be allocated in this function.
 *
 * @pth: The absolute path to the png-file
 * @buf: The buffer to write the pixel-data to
 * @w: The pointer to write the width to
 * @h: The pointer to write the height to
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int texLoadPNG(char *pth, uint8_t **buf, int *w, int *h)
{
	int width, height, size;
	SDL_Surface *surf;
	uint8_t *buffer;

	surf = IMG_Load(pth);
	if(surf == NULL) {
		return(-1);
	}

	width = surf->w;
	height = surf->h;
	size = surf->pitch * surf->h;

	buffer = malloc(size);
	if(buffer == NULL) return(-1);

	memcpy(buffer, surf->pixels, size);

	*buf = buffer;
	*w = width;
	*h = height;

	return(0);
}
