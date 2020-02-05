#ifndef _TEX_UTILS_H_
#define _TEX_UTILS_H_

#include <stdint.h>

/* Load an PNG-file into a pixel-buffer */
int texLoadPNG(char *pth, uint8_t **buf, int *w, int *h);

#endif
