#ifndef _UTILS_H
#define _UTILS_H

#include "core.h"

#include <stdint.h>

extern unsigned long hash(char *key, int len, long lim);

extern int fs_load_png(char *pth, uint8_t **buf, int *w, int *h);

extern int fs_load_file(char *pth, uint8_t **buf, long *len);

#endif
