#include "asset.h"
#include "error.h"
#include "utf8.h"
#include "utils.h"

#include <stdlib.h>


/* Redefine global asset-wrapper */
struct ast_wrapper g_ast;


extern int ast_init(void)
{
	short i;

	/* Initialize shader-table */
	for(i = 0; i < SHD_SLOTS; i++)
		g_ast.shd.mask[i] = 0;

	/* Initialize texture-table */
	for(i = 0; i < TEX_SLOTS; i++)
		g_ast.tex.mask[i] = 0;

	/* Initialize the font-table */
	g_ast.txt.font_num = 0;
	for(i = 0; i < TXT_FONT_SLOTS; i++)
		g_ast.txt.fonts[i] = NULL;

	return 0;
}


extern void ast_close(void)
{
	short i;


	/* Close the font-table */
	for(i = 0; i < TXT_FONT_SLOTS; i++) {
		if(g_ast.txt.fonts[i] != NULL)
			TTF_CloseFont(g_ast.txt.fonts[i]);
	}

	/* Close the texture-table */
	for(i = 0; i < TEX_SLOTS; i++) {
		if(g_ast.tex.mask[i] == 0)
			continue;
		
		ren_destroy_texture(g_ast.tex.hdl[i], g_ast.tex.tex[i]);
	}

	/* Close the shader-table */
	for(i = 0; i < SHD_SLOTS; i++) {
		if(g_ast.shd.mask[i] == 0)
			continue;

		ren_destroy_shader(g_ast.shd.prog[i], g_ast.shd.pipeline[i]);
	}
}


static short shd_get_slot(void)
{
	short i;

	for(i = 0; i < SHD_SLOTS; i++) {
		if(g_ast.shd.mask[i] == 0)
			return i;
	}

	return -1;
}


static int shd_check_slot(short slot)
{
	if(slot < 0 || slot >= SHD_SLOTS)
		return 1;

	return 0;
}


extern short shd_set(char *name, char *vs, char *fs, int num, char **vars, enum mdl_type type)
{
	short slot;

	if((slot = shd_get_slot()) < 0) {
		ERR_LOG(("Shader-table already full"));
		return -1;
	}

	if(!vs || !fs)
		return -1;

	if(ren_create_shader(vs, fs, &g_ast.shd.prog[slot],
					&g_ast.shd.pipeline[slot], num, vars, type) < 0)
		return -1;

	g_ast.shd.mask[slot] = 1;
	strcpy(g_ast.shd.name[slot], name);
	return slot;
}


extern void shd_del(short slot)
{
	if(shd_check_slot(slot))
		return;

	if(g_ast.shd.mask[slot] == 0)
		return;

	ren_destroy_shader(g_ast.shd.prog[slot], g_ast.shd.pipeline[slot]);
	g_ast.shd.mask[slot] = 0;
}


extern short shd_get(char *name)
{
	int i;

	for(i = 0; i < SHD_SLOTS; i++) {
		if(g_ast.shd.mask[i] == 0)
			continue;

		if(!strcmp(g_ast.shd.name[i], name))
			return i;
	}

	return -1;
}


extern void shd_use(short slot, int attr)
{

	if(shd_check_slot(slot))
		return;

	ren_set_shader(g_ast.shd.prog[slot], attr, g_ast.shd.pipeline[slot]);
}


extern void shd_unuse(void)
{
	glUseProgram(0);
}



static short tex_get_slot(void)
{
	short i;

	for(i = 0; i < TEX_SLOTS; i++) {
		if(g_ast.tex.mask[i] == 0)
			return i;
	}

	return -1;
}


static int tex_check_slot(short slot)
{
	if(slot < 0 || slot >= TEX_SLOTS)
		return 1;

	return 0;
}


extern short tex_set(char *name, char *pth)
{
	short slot;

	if((slot = tex_get_slot()) < 0) {
		ERR_LOG(("Texture-table already full"));
		return -1;
	}

	if(ren_create_texture(pth, &g_ast.tex.hdl[slot], &g_ast.tex.tex[slot])
					< 0)
		return -1;

	g_ast.tex.mask[slot] = 1;
	strcpy(g_ast.tex.name[slot], name);
	return slot;
}


extern short skybox_set(char *name, char *pths[6])
{
	short slot;

	if((slot = tex_get_slot()) < 0) {
		ERR_LOG(("Texture-table already full"));
		return -1;
	}

	if(ren_create_skybox(pths, &g_ast.tex.hdl[slot], &g_ast.tex.tex[slot])
				< 0)
		return -1;

	g_ast.tex.mask[slot] = 1;
	strcpy(g_ast.tex.name[slot], name);
	return slot;
}


extern void tex_del(short slot)
{
	if(tex_check_slot(slot))
		return;

	if(g_ast.tex.mask[slot] == 0)
		return;

	ren_destroy_texture(g_ast.tex.hdl[slot], g_ast.tex.tex[slot]);
	g_ast.tex.mask[slot] = 0;
}


extern short tex_get(char *name)
{
	int i;

	for(i = 0; i < TEX_SLOTS; i++) {
		if(g_ast.tex.mask[i] == 0)
			continue;

		if(!strcmp(g_ast.tex.name[i], name))
			return i;
	}

	return -1;
}


extern void tex_use(short slot)
{
	if(tex_check_slot(slot))
		return;

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_ast.tex.hdl[slot]);
}


extern void tex_unuse(void)
{
	glBindTexture(GL_TEXTURE_2D, 0);
}



extern short txt_load_ttf(char *pth, int size)
{
	font_t *font;

	if(g_ast.txt.font_num >= (TXT_FONT_SLOTS - 1)) {
		ERR_LOG(("Font-table is already full"));
		return -1;
	}

	if(!(font = TTF_OpenFont(pth, size))) {
		ERR_LOG(("Failed to load font %s", pth));
		return -1;
	}

	g_ast.txt.fonts[g_ast.txt.font_num] = font;
	g_ast.txt.font_num++;
	return g_ast.txt.font_num - 1;
}


extern void txt_render_rel(surf_t *surf, rect_t *rect, color_t *col, short font,
		char *text, short rel, uint8_t algn)
{
	char subs[512];
	int relw;
	int relh;
	int reloff;
	surf_t *rend;
	surf_t *clipped;
	rect_t clip;
	rect_t out;
	rect_t final;

	if(strlen(text) < 1)
		return;

	TTF_SetFontKerning(g_ast.txt.fonts[font], 0);
	TTF_SetFontHinting(g_ast.txt.fonts[font], TTF_HINTING_NORMAL);

	if(!(rend = TTF_RenderUTF8_Blended(g_ast.txt.fonts[font], text, *col)))
		return;

	reloff = u8_offset(text, rel);
	memcpy(subs, text, reloff);
	subs[reloff] = '\0';
	TTF_SizeUTF8(g_ast.txt.fonts[font], subs, &relw, &relh);

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


extern void txt_render(surf_t *surf, rect_t *rect, color_t *col, short font,
		char *text, uint8_t opt)
{
	int delx;
	int dely;
	int x;
	rect_t text_rect;
	rect_t src_rect;
	surf_t *rend;
	font_t *f_ptr;

	if(text == NULL || strlen(text) <= 0)
		return;

	text[strlen(text)] = 0;

	if(!(f_ptr = g_ast.txt.fonts[font]))
		return;

	TTF_SetFontKerning(f_ptr, 0);
	TTF_SetFontHinting(f_ptr, TTF_HINTING_NORMAL);

	if(!(rend = TTF_RenderUTF8_Blended(f_ptr, text, *col)))
		return;

	delx = rect->w - rend->w;
	dely = rect->h - rend->h;

	x = floor(delx / 2);
	/* TXT_LEFT */
	if((opt >> 0) & 1) {
		x = 0;
	}
	/* TXT_RIGHT */
	if((opt >> 1) & 1) {
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
