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


/* Define the global font-wrapper instance */
extern struct text_wrapper texts;


/*
 * Initialize the font-wrapper.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int txt_init(void);


/*
 * Close the font-wrapper, remove all fonts and free the allocated memory.
 */
extern void txt_close(void);


/*
 * Load a TTF-font and add it to the font-table.
 *
 * @pth: The relative path to the font
 * @size: The size to load the font at
 * 
 * Returns: Either the slot of the font in the table or -1 if an error occurred
 */
extern short txt_font_ttf(char *pth, int size);


/*
 * Render a text on a SDL-surface.
 *
 * @surf: The SDL-surface to render on
 * @rect: The rectangle to render the text in
 * @col: The color to render the text with
 * @font: The slot of the font in the table
 * @text: A null-terminated text-buffer
 * @rel: The relative offset to render the text with
 * @algn: The alignment of the text
 */
extern void txt_render_rel(SDL_Surface *surf, SDL_Rect *rect, SDL_Color *col,
		short font, char *text, short rel, uint8_t algn);


/*
 * Render a text on a surface.
 *
 * @surf: The SDL-surface to render on
 * @rect: The rectangle to render the text in
 * @col: The color to render the text with
 * @font: The slot of the font in the table
 * @text: A null-terminated text-buffer
 * @opt: Additional options
 */
extern void txt_render(SDL_Surface *surf, SDL_Rect *rect, SDL_Color *col,
		short font, char *text, uint8_t opt);

#endif
