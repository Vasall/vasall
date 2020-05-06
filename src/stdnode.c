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

const struct ui_node_flags WRAPPER_FLAGS = {
	1, 0, 0, CURSOR_ARROW
};

const struct ui_node_style WRAPPER_STYLE = {
	0, 0, {0,0,0,0}, 0, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, 0, 0
};

const struct ui_node_style TEXT_STYLE = {
	1, 0, {0,0,0,0}, 0, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, 0, 0
};

void TEXT_RENDER(ui_node *n, ui_node *rel)
{
	ui_text *ele;
	SDL_Rect rect;

	ele = (ui_text *)n->element;

	memcpy(&rect, &n->rel, sizeof(SDL_Rect));
	rect.x -= rel->abs.x;
	rect.y -= rel->abs.y;

	txt_render(rel->surf, &rect, &ele->col, ele->font,
			ele->text, ele->opt);

}

const struct ui_node_flags BUTTON_FLAGS = {
	1, 1, 0, CURSOR_HAND
};

const struct ui_node_style BUTTON_STYLE = {
	1, 1, {252,252,252,255}, 1, {0,0,0,255}, {0,0,0,0}, {0,0,0,0}, 0, 0
};

const struct ui_node_flags INPUT_FLAGS = {
	1, 1, 1, CURSOR_IBEAM
};

const struct ui_node_style INPUT_STYLE = {
	1, 1, {255,255,255,255}, 1, {0,0,0,255}, {0,0,0,0}, {5,5,5,5}, 0, 0
};

void INPUT_ONFOCUS(ui_node *node, SDL_Event *evt)
{
	ui_input *ele = node->element;

	if(evt){/* Prevent warnings for not using parameters */}

	ele->cur = 1;
	SDL_StartTextInput();
}

void INPUT_ONUNFOCUS(ui_node *node, SDL_Event *evt)
{
	ui_input *ele = node->element;

	if(evt){/* Prevent warnings for not using parameters */}

	ele->cur = 0;
	SDL_StopTextInput();
}

void INPUT_ONACTIVE(ui_node *n, SDL_Event *evt)
{
	time_t rawtime;
	ui_input *ele = n->element;
	uint8_t bef = ele->cur;

	if(evt){/* Prevent warning for not using parameters */}

	time(&rawtime);
	ele->cur = ((rawtime % 2) == 0);

	if(bef != ele->cur) {
		ui_prerender(n);
	}
}

void INPUT_ONKEYDOWN(ui_node *node, SDL_Event *evt)
{
	ui_input *ele;
	SDL_Rect *rect;
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

	ui_prerender(node);
}

void INPUT_ONTEXTINPUT(ui_node *node, SDL_Event *evt)
{
	int curw, curh, relw, relh, curoff, reloff;
	char subs[512];
	ui_input *ele;
	SDL_Rect *rect;
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

	if(u8_strlen(ele->buffer) >= ele->limit - 1) {
		return;
	}

	if(u8_strins(ele->buffer, evt->text.text, ele->pos) < 0) {
		return;
	}

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

	ui_prerender(node);
}

void INPUT_RENDER(ui_node *n, ui_node *rel)
{
	int curw, curh, relw, relh, curoff, reloff;
	char subs[512];
	ui_input *ele;
	TTF_Font *font;
	SDL_Rect rect;

	ele = n->element;
	font = texts.fonts[1];

	memcpy(&rect, &n->body, sizeof(SDL_Rect));

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

ui_node *ui_add_wrapper(ui_node *par, char* id, int x, int y,
		int w, int h)
{
	ui_node *ele = NULL, *node = NULL;
	SDL_Rect body;

	body.x = x;
	body.y = y;
	body.w = w;
	body.h = h;

	if(!(node = ui_add(par, NULL, UI_WRAPPER, id, &body)))
		goto err_free_ele;

	node->flags = WRAPPER_FLAGS;
	node->style = WRAPPER_STYLE;
	return node;

err_free_ele:
	free(ele);
	return NULL;
}

ui_node *ui_add_text(ui_node *par, char* id, SDL_Rect *body,
		char *text, SDL_Color *col, uint8_t font, uint8_t opt)
{
	ui_text *ele = NULL;
	SDL_Rect txt_body;
	ui_node *node;
	char *buf = NULL;

	if(!(ele = malloc(sizeof(ui_text))))
		return NULL;

	if(!(ele->text = malloc(strlen(text) * sizeof(char))))
		goto err_free_ele;

	strcpy(ele->text, text);
	memcpy(&ele->col, col, sizeof(SDL_Color));
	ele->font = font;
	ele->opt = opt;

	if(!(node = ui_add(par, ele, UI_TEXT, id, body)))
		goto err_free_text;

	node->flags = NULL_FLAGS;
	node->style = TEXT_STYLE;
	node->render = &TEXT_RENDER;
	return node;

err_free_text:
	free(ele->text);

err_free_ele:
	free(ele);
	return NULL;
}

ui_node *ui_add_button(ui_node *par, char* id, int x, int y,
		int w, int h, char *label)
{
	ui_button *ele = NULL;
	SDL_Rect body;
	ui_node *node;
	char label_id[256];

	if(!(ele = malloc(sizeof(ui_button))))
		return NULL;

	body.x = x;
	body.y = y;
	body.w = w;
	body.h = h;

	if(!(node = ui_add(par, ele, UI_BUTTON, id, &body)))
		goto err_free_ele;

	node->flags = BUTTON_FLAGS;
	node->style = BUTTON_STYLE;

	body.x = 0;
	body.y = 0;

	/* TODO: check if creating text failed */
	sprintf(label_id, "%s_%s", id, "label");
	ui_add_text(node, label_id, &body, label, (SDL_Color *)&BLACK, 1, 0);
	return node;

err_free_ele:
	free(ele);
	return NULL;
}

ui_node *ui_add_input(ui_node *par, char* id, int x, int y,
		int w, int h, char *plhdr)
{
	ui_input *ele;
	SDL_Color text_color = {0,0,0,255};
	SDL_Color cursor_color = {0x00, 0x00, 0x00, 0xff};
	SDL_Rect body;
	ui_node *node;

	if(plhdr) {/* Prevent warning for not using plhdr */}

	if(!(ele = malloc(sizeof(ui_input))))
		return NULL;

	memcpy(&ele->col, &text_color, sizeof(SDL_Color));

	ele->pos = 0;
	ele->cur = 0;
	memcpy(&ele->cur_col, &cursor_color, sizeof(SDL_Color));

	ele->algn = 0;
	ele->rel = 0;

	ele->actime = -1;

	ele->buffer[0] = 0;
	ele->limit = 128;
	ele->hide = 0;

	body.x = x;
	body.y = y;
	body.w = w;
	body.h = h;

	if(!(node = ui_add(par, ele, UI_INPUT, id, &body)))
		goto err_free_ele;

	node->flags = INPUT_FLAGS;
	node->style = INPUT_STYLE;

	node->events.focus = &INPUT_ONFOCUS;
	node->events.unfocus = &INPUT_ONUNFOCUS;
	node->events.onactive = &INPUT_ONACTIVE;
	node->events.keydown = &INPUT_ONKEYDOWN;
	node->events.textinput = &INPUT_ONTEXTINPUT;
	node->render = &INPUT_RENDER;
	return node;

err_free_ele:
	free(ele);
	return NULL;
}
