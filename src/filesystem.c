#include "filesystem.h"
#include "sdl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int fs_load_file(char *pth, uint8_t **buf, long *len)
{
	FILE *fd;
	long length;
	uint8_t *data;

	if(!(fd = fopen(pth, "rb")))
		return -1;

	fseek(fd, 0, SEEK_END);
	length = ftell(fd);
	
	if(!(data = malloc(length + 1)))
		goto err_close_fd;

	fseek(fd, 0, SEEK_SET);
	fread(data, length, 1, fd);

	fclose(fd);
	data[length] = 0;
	*buf = data;
	if(len) *len = length;
	return 0;

err_close_fd:
	fclose(fd);
	return -1;
}

extern int fs_load_png(char *pth, uint8_t **buf, int *w, int *h)
{
	int width;
	int height;
	int size;
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
	SDL_FreeSurface(surf);

	*buf = buffer;
	*w = width;
	*h = height;
	return 0;
}


