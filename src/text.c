#include "text.h"
#include "utf8.h"
#include "utils.h"
#include "error.h"

#include <stdlib.h>


/* Redefine the global text-wrapper */
struct text_wrapper texts;


extern int txt_init(void)
{
	int i;

	texts.font_num = 0;
	for(i = 0; i < FONT_NUM;i++)
		texts.fonts[i] = NULL;

	return 0;
}


extern void txt_close(void)
{
	int i;

	for(i = 0; i < FONT_NUM; i++) {
		if(texts.fonts[i] != NULL)
			TTF_CloseFont(texts.fonts[i]);
	}
}


extern short txt_font_ttf(char *pth, int size)
{
	TTF_Font *font;

	if(texts.font_num >= (FONT_NUM - 1)) {
		ERR_LOG(("Font-table is already full"));
		return -1;
	}

	if(!(font = TTF_OpenFont(pth, size))) {
		ERR_LOG(("Failed to load font %s", pth));
		return -1;
	}

	texts.fonts[texts.font_num] = font;
	texts.font_num++;
	return texts.font_num - 1;

}


extern void txt_render_rel(SDL_Surface *surf, SDL_Rect *rect, SDL_Color *col,
		short font, char *text, short rel, uint8_t algn)
{
	char subs[512];
	int relw;
	int relh;
	int reloff;
	SDL_Surface *rend;
	SDL_Surface *clipped;
	SDL_Rect clip;
	SDL_Rect out;
	SDL_Rect final;

	if(strlen(text) < 1)
		return;

	TTF_SetFontKerning(texts.fonts[font], 0);
	TTF_SetFontHinting(texts.fonts[font], TTF_HINTING_NORMAL);

	if(!(rend = TTF_RenderUTF8_Blended(texts.fonts[font], text, *col)))
		return;

	reloff = u8_offset(text, rel);
	memcpy(subs, text, reloff);
	subs[reloff] = '\0';
	TTF_SizeUTF8(texts.fonts[font], subs, &relw, &relh);

	/* Clip the surface to fit into the specified rect */
	clip.x = 0;
	clip.y = 0;
	clip.w = rend->w;
	clip.h = rend->h;

	/* Align text left */
	if(algn == 0) {
		out.x = -relw;
	}
	/* Align text right */
	if(algn == 1) {
		out.x = rect->w - relw;
	}
	out.y = (rect->h - clip.h) / 2;
	out.w = rect->w;
	out.h = rect->h;

	clipped = crop_surf(rend, &clip, &out);

	final.x = 0;
	final.y = 0;
	final.w = clipped->w;
	final.h = clipped->h;

	SDL_BlitSurface(clipped, &final, surf, rect);

	SDL_FreeSurface(clipped);
	SDL_FreeSurface(rend);
}


extern void txt_render(SDL_Surface *surf, SDL_Rect *rect, SDL_Color *col,
		short font, char *text, uint8_t opt)
{
	int delx;
	int dely;
	int x;
	SDL_Rect text_rect;
	SDL_Rect src_rect;
	SDL_Surface *rend;
	TTF_Font *f_ptr;

	if(text == NULL || strlen(text) <= 0)
		return;

	f_ptr = texts.fonts[font];

	TTF_SetFontKerning(f_ptr, 0);
	TTF_SetFontHinting(f_ptr, TTF_HINTING_NORMAL);

	rend = TTF_RenderUTF8_Blended(f_ptr, text, *col);

	delx = rect->w - rend->w;
	dely = rect->h - rend->h;

	x = floor(delx / 2);
	/* XSDL_TXT_LEFT */
	if(((opt >> 0) & 1) == 1) {
		x = 0;
	}
	/* XSDL_TXT_RIGHT */
	if(((opt >> 1) & 1) == 1) {
		x = rect->w - rend->w;
	}
	
	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.w = rend->w;
	src_rect.h = rend->h;

	text_rect.x = x;
	text_rect.y = floor(dely / 2);
	text_rect.w = rend->w;
	text_rect.h = rend->h;

	if(rend->w > rect->w) {
		text_rect.x = rect->w - rend->w;
	}

	text_rect.x += rect->x;
	text_rect.y += rect->y;

	SDL_BlitSurface(rend, &src_rect, surf, &text_rect);
	SDL_FreeSurface(rend);
}
