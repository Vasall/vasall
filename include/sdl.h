#ifndef _SDL_H
#define _SDL_H

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define SDL_R_MASK 0xff000000
#define SDL_G_MASK 0x00ff0000
#define SDL_B_MASK 0x0000ff00
#define SDL_A_MASK 0x000000ff
#else
#define SDL_R_MASK 0x000000ff
#define SDL_G_MASK 0x0000ff00
#define SDL_B_MASK 0x00ff0000
#define SDL_A_MASK 0xff000000
#endif

extern int sdl_init(void);
extern void sdl_close(void);

extern int sdl_fill_rounded(SDL_Surface *surf, int xo, int yo, int w, int h, 
		SDL_Color col, short *cor);
#endif
