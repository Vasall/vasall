#ifndef _TEXT_H
#define _TEXT_H

#include "sdl.h"

#define FONT_NUM 12

/*
 *  128  64  32  16   8   4   2   1
 *    7   6   5   4   3   2   1   0
 *
 * 0: Align text left
 * 1: Align text right
 * 2: UNDEFINED
 * 3: UNDEFINED
 * 4: Bold
 * 5: Italic
 * 6: Underline
 */

#define TEXT_LEFT       0x01
#define TEXT_RIGHT	0x02
#define TEXT_BOLD       0x10
#define TEXT_ITALIC     0x20
#define TEXT_UNDERLINE  0x40

struct text_wrapper {
	short font_num;
	TTF_Font *fonts[FONT_NUM];
};


extern struct text_wrapper texts;


extern int txt_init(void);
extern void txt_close(void);

extern short txt_font_ttf(char *pth, int size);

extern void txt_render_rel(SDL_Surface *surf, SDL_Rect *rect, SDL_Color *col,
		short font, char *text, short rel, uint8_t algn);
extern void txt_render(SDL_Surface *surf, SDL_Rect *rect, SDL_Color *col,
		short font, char *text, uint8_t opt);

#endif
