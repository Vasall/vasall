#include "sdl.h"

#include "error.h"
#include "extmath.h"

#include <stdio.h>
#include <stdlib.h>


extern int sdl_init(void)
{
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		ERR_LOG(("Failed to initialize SDL-subsystem"));
		return -1;
	}

	if(TTF_Init() < 0) {
		ERR_LOG(("Failed to initialize TTF-subsystem"));
		goto err_quit_sdl;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	return 0;

err_quit_sdl:
	SDL_Quit();
	return -1;
}


extern void sdl_close(void)
{
	if(TTF_WasInit())
		TTF_Quit();

	SDL_Quit();
}


extern void sdl_print_info(void)
{
	SDL_version sdl_comp;
	SDL_version sdl_link;
	SDL_version img_comp;
	const SDL_version *img_link;
	SDL_version ttf_comp;

	/* Get the TTF version */
	const SDL_version *ttf_link = TTF_Linked_Version();
	SDL_TTF_VERSION(&ttf_comp);

	/* Get the Image version */
	img_link = IMG_Linked_Version();
	SDL_IMAGE_VERSION(&img_comp);

	/* Get the SDL version */
	SDL_VERSION(&sdl_comp);
	SDL_GetVersion(&sdl_link);

	printf("-------------------- SDL Info --------------------\n");
	printf("SDL comp. Version: %d.%d.%d\n", 
			sdl_comp.major, sdl_comp.minor, sdl_comp.patch);
	printf("SDL link. Version: %d.%d.%d\n", 
			sdl_link.major, sdl_link.minor, sdl_link.patch);

	printf("IMG comp. Version: %d.%d.%d\n",
			img_comp.major, img_comp.minor, img_comp.patch);
	printf("IMG link. Version: %d.%d.%d\n",
			img_link->major, img_link->minor, img_link->patch);

	printf("TTF comp. Version: %d.%d.%d\n", 
			ttf_comp.major, ttf_comp.minor, ttf_comp.patch);
	printf("TTF link. Version: %d.%d.%d\n", 
			ttf_link->major, ttf_link->minor, ttf_link->patch);
	printf("--------------------------------------------------\n");
}


extern void sdl_lock_mouse(enum sdl_mouse_mode mode)
{
	SDL_bool f;

	if(mode == SDL_MOUSE_LOCK)
		f = SDL_TRUE;
	else
		f = SDL_FALSE;

	SDL_SetRelativeMouseMode(f);
}


extern int sdl_fill_rounded(surf_t *surf, int xo, int yo, int w, int h,
		color_t col, short *cor)
{
	uint32_t *pixels;
	float a, d, f;
	int i, j, x, y;
	color_t cur, set;

	xo = (xo < 0) ? (0) : (xo);
	yo = (yo < 0) ? (0) : (yo);
	w = (xo + w >= surf->w) ? (surf->w - xo) : (w);
	h = (yo + h >= surf->h) ? (surf->h - yo) : (h);

	if(SDL_LockSurface(surf) < 0) {
		ERR_LOG(("Failed to lock surface"));
		return -1;
	}

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
			else if(a > 127.5) {
				cur =  *((color_t *)&pixels[y * surf->w + x]);
				f = cur.a + a;

				set.r = cur.r * (cur.a / f) + col.r * (a / f);
				set.g = cur.g * (cur.a / f) + col.g * (a / f);
				set.b = cur.b * (cur.a / f) + col.b * (a / f);
				set.a = 255;		
			}

			if(a > 127.5) {
				pixels[(y * surf->w) + x] = *((uint32_t *)&set);
			}
		}
	}

	SDL_UnlockSurface(surf);
	return  0;
}


extern rect_t sdl_rect(int x, int y, int w, int h)
{
	static rect_t r;

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;

	return r;
}


extern color_t sdl_color(int r, int g, int b, int a)
{
	static color_t col;

	col.r = r;
	col.g = g;
	col.b = b;
	col.a = a;

	return col;
}


extern rect_t *sdl_rect_s(int x, int y, int w, int h)
{
	static rect_t r;

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;

	return &r;
}


extern color_t *sdl_color_s(int r, int g, int b, int a)
{
	static color_t col;

	col.r = r;
	col.g = g;
	col.b = b;
	col.a = a;

	return &col;
}
