#ifndef _SDL_H
#define _SDL_H

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

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


typedef SDL_Color color_t;
typedef SDL_Rect rect_t;
typedef SDL_Event event_t;
typedef SDL_Surface surf_t;

typedef TTF_Font font_t;

/*
 * Initialize the SDL-framework and it's submodules.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int sdl_init(void);


/*
 * Shutdown the SDL-framework and it's submodules.
 */
extern void sdl_close(void);


/*
 * Print usefull information about the SDL-subsystems in the console, which can
 * then be used for debugging purposes.
 */
extern void sdl_print_info(void);


enum sdl_mouse_mode {
	SDL_MOUSE_LOCK,
	SDL_MOUSE_FREE
};

extern void sdl_lock_mouse(enum sdl_mouse_mode mode);

/*
 * Render a rectangle with rounded corners onto a SDL-surface,
 *
 * @surf: A pointer to the SDL-surface
 * @xo: The x-position on the surface
 * @yo: The y-position on the surface
 * @w: The width of the ractangle
 * @h: The height of the rectangle
 * @col: The color of the rectangle
 * @cor: An array containing the radii of the corners
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int sdl_fill_rounded(surf_t *surf, int xo, int yo, int w, int h,
		color_t col, short *cor);


extern rect_t sdl_rect(int x, int y, int w, int h);

extern color_t sdl_color(int r, int g, int b, int a);

extern rect_t *sdl_rect_s(int x, int y, int w, int h);

extern color_t *sdl_color_s(int r, int g, int b, int a);

#endif
