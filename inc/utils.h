#ifndef _UTILS_H
#define _UTILS_H

#include "sdl.h"


extern int strins(char *buf, char *ins, int pos);


extern char *get_bin_dir(char *pth);


void join_paths(char *dst, char *pth1, char *pth2);


extern surf_t *crop_surf(surf_t* in, rect_t *in_rect, rect_t *out_rect);

#endif
