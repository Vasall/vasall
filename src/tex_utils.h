#ifndef _V_TEX_UTILS_H
#define _V_TEX_UTILS_H

#include "defines.h"
#include "core.h"

#include <stdint.h>

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
V_API int tex_load_png(char *pth, uint8_t **buf, int *w, int *h);

#endif
