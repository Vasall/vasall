#include "tex_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
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
	int width, height, y, i, j, k, c = 0;
	png_byte color_type;
	png_byte bit_depth;
	png_bytep *row_pointers = NULL;
	FILE *fp;
	png_structp png;
	png_infop info;
	uint8_t *buffer;

	fp = fopen(pth, "rb");
	if(fp == NULL) goto failed;
		
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png == NULL) goto failed;
	
	info = png_create_info_struct(png);
	if(info == NULL) goto failed;

	if(setjmp(png_jmpbuf(png))) goto failed;

	png_init_io(png, fp);

	png_read_info(png, info);

	width      = png_get_image_width(png, info);
	height     = png_get_image_height(png, info);
	color_type = png_get_color_type(png, info);
	bit_depth  = png_get_bit_depth(png, info);

	if(bit_depth == 16) {
		png_set_strip_16(png);
	}

	if(color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png);
	}

	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
		png_set_expand_gray_1_2_4_to_8(png);
	}

	if(png_get_valid(png, info, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png);
	}

	if(color_type == PNG_COLOR_TYPE_RGB ||
			color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	}

	if(color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png);
	}

	png_read_update_info(png, info);

	if(row_pointers) goto failed;

	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(y = 0; y < height; y++) {
		row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	}

	png_read_image(png, row_pointers);

	buffer = malloc(width * height * 3);
	if(buffer == NULL) goto failed;

	for(i = 0; i < height; i++) {
		png_byte *row = row_pointers[i];

		for(j = 0; j < width; j++) {
			png_byte *ptr = &(row[j * 4]);
	
			for(k = 0; k < 3; k++) {
				buffer[c++] = ptr[k];
			}
		}
	}	

	*buf = buffer;
	*w = width;
	*h = height;

	fclose(fp);

	png_destroy_read_struct(&png, &info, NULL);

	return(0);

failed:
	return(-1);
}
