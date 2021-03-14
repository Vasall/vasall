#include "filesystem.h"
#include "error.h"
#include "sdl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


extern int fs_load_file(char *pth, uint8_t **buf, long *len)
{
	FILE *fd;
	long length;
	uint8_t *data;

	if(!(fd = fopen(pth, "rb"))) {
		ERR_LOG(("Failed to open file: %s", pth));
		return -1;
}

	fseek(fd, 0, SEEK_END);
	length = ftell(fd);
	
	if(!(data = malloc(length + 1))) {
		ERR_LOG(("Failed to allocate memory"));
		goto err_close_fd;
	}

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

	if(!(surf = IMG_Load(pth))) {
		ERR_LOG(("Failed to load image: %s", pth));
		return -1;
}

	width = surf->w;
	height = surf->h;
	size = surf->pitch * surf->h;

	if(!(buffer = malloc(size))) {
		ERR_LOG(("Failed to allocate memory"));
		return -1;
	}

	memcpy(buffer, surf->pixels, size);
	SDL_FreeSurface(surf);

	*buf = buffer;
	*w = width;
	*h = height;
	return 0;
}

extern int fs_write_file(char *pth, uint8_t *buf, long length)
{
	FILE *fd;

	if(!(fd = fopen(pth, "wb"))) {
		ERR_LOG(("Failed to open file: %s", pth));
		return -1;
	}

	fwrite(buf, length, 1, fd);

	fclose(fd);
	return 0;
}


extern int fs_create_dir(char *pth)
{
	struct stat st;

	if(stat(pth, &st) == 0) {
		return -1;
	}

	if(mkdir(pth, 0755) < 0) {
		ERR_LOG(("Failed to create directory: %s", pth));
		return -1;
	}

	return 0;
}