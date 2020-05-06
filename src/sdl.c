#include "sdl.h"
#include "mbasic.h"

#include <stdio.h>
#include <stdlib.h>

extern int sdl_init(void)
{
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return -1;

	if(TTF_Init() < 0)
		goto err_quit_sdl;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_PROFILE_CORE);

	return 0;

err_quit_sdl:
	SDL_Quit();
	return -1;
}

extern void sdl_close(void)
{
	if(TTF_WasInit()) {
		TTF_Quit();
	}

	SDL_Quit();
}

extern int sdl_fill_rounded(SDL_Surface *surf, int xo, int yo, int w, int h, 
		SDL_Color col, short *cor)
{
	uint32_t *pixels;
	float a, d, f;
	int i, j, x, y;
	SDL_Color cur, set;

	xo = (xo < 0) ? (0) : (xo);
	yo = (yo < 0) ? (0) : (yo);
	w = (xo + w >= surf->w) ? (surf->w - xo) : (w);
	h = (yo + h >= surf->h) ? (surf->h - yo) : (h);

	if(SDL_LockSurface(surf) < 0)
		return -1;

	pixels = (uint32_t *)surf->pixels;

	for(i = 0; i < h; i++) {
		for(j = 0; j < w; j++) {
			x = j + xo;
			y = i + yo;

			a = 1.0;
			if(cor[0] && j <= cor[0] && i <= cor[0]) {
				d = dist(j + 1, i + 1, cor[0], cor[0]);
				a = clamp(cor[0] - d);
			} 
			else if(cor[3] && j <= cor[3] && i >= h - cor[3]) {
				d = dist(j + 1, i, cor[3], h - cor[3]);
				a = clamp(cor[3] - d);
			} 
			else if(cor[1] && j >= w - cor[1] && i <= cor[1]) {
				d = dist(j, i + 1, w - cor[1], cor[1]);
				a = clamp(cor[1] - d);
			} 
			else if(cor[2] && j >= w - cor[2] && i >= h - cor[2]) {
				d = dist(j, i, w - cor[2], h - cor[2]);
				a = clamp(cor[2] - d);
			}

			a = a * col.a;
			if(a == 255.0) {
				set.r = col.r;
				set.g = col.g;
				set.b = col.b;
				set.a = 255;
			}
			else if(a > 0.0) {
				cur =  *((SDL_Color *)&pixels[y * surf->w + x]);
				f = cur.a + a;

				set.r = cur.r * (cur.a / f) +
					col.r * (a / f);
				set.g = cur.g * (cur.a / f) +
					col.g * (a / f);
				set.b = cur.b * (cur.a / f) +
					col.b * (a / f);
				set.a = 255;		
			}

			if(a > 0.0) {
				pixels[(y * surf->w) + x] = *((uint32_t *)&set);
			}
		}
	}

	SDL_UnlockSurface(surf);
	return  0;
}
