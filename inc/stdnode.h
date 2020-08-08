#ifndef _STDNODE_H
#define _STDNODE_H

#include "sdl.h"
#include "text.h"
#include "node.h"
#include "window.h"

extern SDL_Color WHITE;
extern SDL_Color BLACK;
extern const char BULLET[3];


/* --------------------- WRAPPER ---------------------- */

extern const struct ui_node_flags WRAPPER_FLAGS;
extern const struct ui_node_style WRAPPER_STYLE;

extern int ui_init_wrapper(ui_node *n);



/* ----------------------- TEXT ----------------------- */

typedef struct ui_text {
	char *text;
	SDL_Color col;
	uint8_t font;
	uint8_t opt;
} ui_text;

extern const struct ui_node_style TEXT_STYLE;

void TEXT_RENDER(ui_node *n, ui_node *rel);

extern void *ui_new_text(char *text, color_t col, uint8_t font, uint8_t opt);
extern int ui_init_text(ui_node *n);



/* ---------------------- BUTTON ---------------------- */

typedef struct ui_button {
	int tmp;
} ui_button;

extern const struct ui_node_flags BUTTON_FLAGS;
extern const struct ui_node_style BUTTON_STYLE;

extern int ui_init_button(ui_node *n);



/* ---------------------- INPUT ----------------------- */

typedef struct ui_input {
	char buffer[129];
	short limit;
	uint8_t algn;
	short rel;
	short pos;
	SDL_Color col;
	uint8_t cur;
	int actime;
	SDL_Color cur_col;
	short sel_str;			
	short sel_end;
	uint8_t hide;
} ui_input;

extern const struct ui_node_flags INPUT_FLAGS;
extern const struct ui_node_style INPUT_STYLE;

void INPUT_ONFOCUS(ui_node *n, event_t *evt);
void INPUT_ONUNFOCUS(ui_node *n, event_t *evt);
void INPUT_ONACTIVE(ui_node *n, event_t *evt);
void INPUT_ONKEYDOWN(ui_node *n, event_t *evt);
void INPUT_ONTEXTINPUT(ui_node *n, event_t *evt);
void INPUT_RENDER(ui_node *n, ui_node *rel);

extern void *ui_new_input(color_t txt_col);
extern int ui_init_input(ui_node *n);

#endif
