#ifndef _UTILS_H
#define _UTILS_H

#include "sdl.h"


extern int strins(char *buf, char *ins, int pos);


extern char *get_bin_dir(char *pth);


void join_paths(char *dst, char *pth1, char *pth2);


extern SDL_Surface *crop_surf(SDL_Surface* in, SDL_Rect *in_rect, 
		SDL_Rect *out_rect);

#endif
