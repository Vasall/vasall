#include "stdnode.h"
#include "utf8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Redefine external variables */
SDL_Color WHITE = { 0xff, 0xff, 0xff, 0xff };
SDL_Color BLACK = { 0x00, 0x00, 0x00, 0x00 };
const char BULLET[3] = {(char)0xE2, (char)0x80, (char)0xA2};



/* =========================================================== *
 *                                                             *
 *                           WRAPPER                           *
 *                                                             *
 * =========================================================== */

const struct ui_node_flags WRAPPER_FLAGS = {
	1, 0, 0, CURSOR_ARROW
};

const struct ui_node_style WRAPPER_STYLE = {
	0, 0, {0,0,0,0}, 0, {0,0,0,0}, {0,0,0,0}
};

extern int ui_init_wrapper(ui_node *n)
{
	n->flags = WRAPPER_FLAGS;
	n->style = WRAPPER_STYLE;
	return 0;
}



/* =========================================================== *
 *                                                             *
 *                             TEXT                            *
 *                                                             *
 * =========================================================== */

const struct ui_node_style TEXT_STYLE = {
	1, 0, {0,0,0,0}, 0, {0,0,0,0}, {0,0,0,0}
};

void TEXT_RENDER(ui_node *n, ui_node *rel)
{
	rect_t rect;
	ui_text *ele;

	ele = (ui_text *)n->element;

	rect.x = n->body.x - rel->body.x;
	rect.y = n->body.y - rel->body.y;

	txt_render(rel->surf, &rect, &ele->col, ele->font, ele->text, ele->opt);
}

extern int ui_init_text(ui_node *n)
{
	n->style = TEXT_STYLE;
	n->render = &TEXT_RENDER;
	return 0;
}


/* =========================================================== *
 *                                                             *
 *                            BUTTON                           *
 *                                                             *
 * =========================================================== */

const struct ui_node_flags BUTTON_FLAGS = {
	1, 1, 0, CURSOR_HAND
};

const struct ui_node_style BUTTON_STYLE = {
	1, 1, {252,252,252,255}, 1, {0,0,0,255}, {0,0,0,0}
};

extern int ui_init_button(ui_node *n)
{
	n->flags = BUTTON_FLAGS;
	n->style = BUTTON_STYLE;
	return 0;
}



/* =========================================================== *
 *                                                             *
 *                             INPUT                           *
 *                                                             *
 * =========================================================== */

const struct ui_node_flags INPUT_FLAGS = {
	1, 1, 1, CURSOR_IBEAM
};

const struct ui_node_style INPUT_STYLE = {
	1, 1, {255,255,255,255}, 1, {0,0,0,255}, {0,0,0,0}
};

void INPUT_ONFOCUS(ui_node *node, event_t *evt)
{
	ui_input *ele = node->element;

	if(evt){/* Prevent warnings for not using parameters */}

	ele->cur = 1;
	SDL_StartTextInput();
}

void INPUT_ONUNFOCUS(ui_node *node, event_t *evt)
{
	ui_input *ele = node->element;

	if(evt){/* Prevent warnings for not using parameters */}

	ele->cur = 0;
	SDL_StopTextInput();
}

void INPUT_ONACTIVE(ui_node *n, event_t *evt)
{
	time_t rawtime;
	ui_input *ele = n->element;
	uint8_t bef = ele->cur;

	if(evt){/* Prevent warning for not using parameters */}

	time(&rawtime);
	ele->cur = ((rawtime % 2) == 0);

	if(bef != ele->cur)
		ui_update(n);
}

void INPUT_ONKEYDOWN(ui_node *node, event_t *evt)
{
	ui_input *ele;
	rect_t *rect;
	TTF_Font *font;
	int txtw, txth, relw, relh, curw, curh, reloff;
	char subs[512];

	ele = node->element;
	rect = &node->body;
	font = texts.fonts[1];

	switch(evt->button.button) {
		case(25):	/* V in combination with CTRL */
			if(SDL_GetModState() & KMOD_CTRL) {
				char *paste_buf;

				paste_buf = SDL_GetClipboardText();

				if(paste_buf == NULL) {
					break;
				}

				if(u8_strins(ele->buffer, paste_buf, ele->pos) < 0) {
					break;
				}

				ele->pos += u8_strlen(paste_buf);
			}
			break;

		case(42):	/* Backspace - delete char */
			if(ele->pos < 1) {
				break;
			}

			u8_strdel(ele->buffer, ele->pos - 1, 1);
			ele->pos--;
			if(ele->algn == 1) {
				ele->rel--;
			}

			TTF_SizeUTF8(font, ele->buffer, &txtw, &txth);

			if(txtw <= rect->w) {
				ele->algn = 0;
				ele->rel = 0;
			}
			break;

		case(74):	/* Pos1 - cursor to beginning of string */
			ele->pos = 0;
			ele->algn = 0;
			ele->rel = 0;
			break;

		case(77):	/* Ende - cursor to end of string */
			ele->pos = u8_strlen(ele->buffer);

			TTF_SizeUTF8(font, ele->buffer, &txtw, &txth);

			if(txtw > rect->w) {
				ele->algn = 1;
				ele->rel = ele->pos;
			}

			break;

		case(79):	/* Right - move one char right */
			if(ele->pos >= u8_strlen(ele->buffer)) {
				break;
			}

			ele->pos++;

			reloff = u8_offset(ele->buffer, ele->pos);
			memcpy(subs, ele->buffer, reloff);
			subs[reloff] = '\0';
			TTF_SizeUTF8(font, subs, &curw, &curh);

			reloff = u8_offset(ele->buffer, ele->rel);
			memcpy(subs, ele->buffer, reloff);
			subs[reloff] = '\0';
			TTF_SizeUTF8(font, subs, &relw, &relh);

			if(ele->algn == 0) {
				if((curw - relw) >= rect->w) {
					ele->algn = 1;
					ele->rel = ele->pos;
				}
			}
			else if(ele->algn == 1 && ele->rel <
					u8_strlen(ele->buffer)) {
				ele->rel++;
			}

			break;

		case(80):	/* Left - move one char left */
			if(ele->pos <= 0) {
				break;
			}

			ele->pos--;

			reloff = u8_offset(ele->buffer, ele->pos);
			memcpy(subs, ele->buffer, reloff);
			subs[reloff] = '\0';
			TTF_SizeUTF8(font, subs, &curw, &curh);

			reloff = u8_offset(ele->buffer, ele->rel);
			memcpy(subs, ele->buffer, reloff);
			subs[reloff] = '\0';
			TTF_SizeUTF8(font, subs, &relw, &relh);

			if(ele->algn == 1) {
				if((relw - curw) >= rect->w) {
					ele->algn = 0;
					ele->rel = ele->pos;
				}
			}
			else if(ele->algn == 0 && (curw- relw) < 0 &&
					ele->rel > 0) {
				ele->rel--;
			}

			break;
	}

	ui_update(node);
}

void INPUT_ONTEXTINPUT(ui_node *node, event_t *evt)
{
	int curw, curh, relw, relh, curoff, reloff;
	char subs[512];
	ui_input *ele;
	rect_t *rect;
	TTF_Font *font;

	ele = node->element;
	rect = &node->body;
	font = texts.fonts[1];

	/* Ignore copy and paste */
	if((SDL_GetModState() & KMOD_CTRL &&
				(evt->text.text[0] == 'c' ||
				 evt->text.text[0] == 'C' ||
				 evt->text.text[0] == 'v' ||
				 evt->text.text[0] == 'V'))) {
		return;
	}

	if(u8_strlen(ele->buffer) >= ele->limit - 1)
		return;

	if(u8_strins(ele->buffer, evt->text.text, ele->pos) < 0)
		return;

	ele->pos++;

	curoff = u8_offset(ele->buffer, ele->pos);
	memcpy(subs, ele->buffer, curoff);
	subs[curoff] = '\0';
	TTF_SizeUTF8(font, subs, &curw, &curh);

	reloff = u8_offset(ele->buffer, ele->rel);
	memcpy(subs, ele->buffer, reloff);
	subs[reloff] = '\0';
	TTF_SizeUTF8(font, subs, &relw, &relh);

	if(ele->algn == 0) {
		if(rect->x + curw - relw >= rect->x + rect->w) {
			ele->algn = 1;
			ele->rel = ele->pos;
		}
	}
	else {
		ele->rel++;
	}

	ui_update(node);
}

void INPUT_RENDER(ui_node *n, ui_node *rel)
{
	int curw, curh, relw, relh, curoff, reloff;
	char subs[512];
	ui_input *ele;
	TTF_Font *font;
	rect_t rect;

	ele = n->element;
	font = texts.fonts[1];

	memcpy(&rect, &n->body, sizeof(rect_t));

	curoff = u8_offset(ele->buffer, ele->pos);
	memcpy(subs, ele->buffer, curoff);
	subs[curoff] = '\0';
	TTF_SizeUTF8(font, subs, &curw, &curh);

	reloff = u8_offset(ele->buffer, ele->rel);
	memcpy(subs, ele->buffer, reloff);
	subs[reloff] = '\0';
	TTF_SizeUTF8(font, subs, &relw, &relh);

	if(ele->algn == 0) curw = rect.x + curw - relw;
	else if(ele->algn == 1) curw = ((rect.x + rect.w) - relw) + curw;

	if(strlen(ele->buffer) > 0) {
		/* TODO: Fix hiding the password */
		if(ele->hide == 1) {
			int i, len, byte_len;
			char bullet_array[385];

			len = u8_strlen(ele->buffer);
			byte_len = (len * 3) + 1;

			for(i = 0; i < (byte_len - 1); i++) {
				bullet_array[i] = BULLET[i % 3];
			}
			bullet_array[i + 1] = 0;

			txt_render_rel(rel->surf, &rect, &ele->col, 1,
					bullet_array, ele->rel, ele->algn);
		}
		else {
			txt_render_rel(rel->surf, &rect, &ele->col, 1,
					ele->buffer, ele->rel, ele->algn);

		}
	}

	if(ele->cur) {
		int lh, iy, *pixels = rel->surf->pixels;

		lh = TTF_FontAscent(font);
		curh = rect.y + (rect.h - lh) / 2;

		SDL_LockSurface(rel->surf);
		for(iy = curh; iy <= curh + lh; iy++) {
			pixels[iy * rel->surf->w + curw] = 0xffffffff;
		}
		SDL_UnlockSurface(rel->surf);

	}
}

extern void *ui_net_input(color_t txt_col)
{
	ui_input *ele;
	color_t cursor_color = {0x00, 0x00, 0x00, 0xff};

	if(!(ele = malloc(sizeof(ui_input))))
		return NULL;

	memcpy(&ele->col, &txt_col, sizeof(color_t));

	ele->pos = 0;
	ele->cur = 0;
	memcpy(&ele->cur_col, &cursor_color, sizeof(SDL_Color));

	ele->algn = 0;
	ele->rel = 0;

	ele->actime = -1;

	ele->buffer[0] = 0;
	ele->limit = 128;
	ele->hide = 0;

	return ele;
}

extern int ui_init_input(ui_node *n)
{
	n->flags = INPUT_FLAGS;
	n->style = INPUT_STYLE;

	n->events.focus = &INPUT_ONFOCUS;
	n->events.unfocus = &INPUT_ONUNFOCUS;
	n->events.onactive = &INPUT_ONACTIVE;
	n->events.keydown = &INPUT_ONKEYDOWN;
	n->events.textinput = &INPUT_ONTEXTINPUT;
	n->render = &INPUT_RENDER;
	return 0;
}
