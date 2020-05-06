#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include <stdint.h>

extern int fs_load_file(char *pth, uint8_t **buf, long *len);

extern int fs_load_png(char *pth, uint8_t **buf, int *w, int *h);

#endif
