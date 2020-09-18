#include "asset.h"
#include "error.h"
#include "utf8.h"
#include "utils.h"

#include <stdlib.h>

struct asset_wrapper assets;


extern int ast_init(void)
{
	short i;

	/* Initialize shader-table */
	for(i = 0; i < SHD_SLOTS; i++)
		assets.shd.mask[i] = 0;

	/* Initialize texture-table */
	for(i = 0; i < TEX_SLOTS; i++)
		assets.tex.mask[i] = 0;

	/* Initialize the font-table */
	assets.txt.font_num = 0;
	for(i = 0; i < TXT_FONT_SLOTS; i++)
		assets.txt.fonts[i] = NULL;

	return 0;
}


extern void ast_close(void)
{
	short i;


	/* Close the font-table */
	for(i = 0; i < TXT_FONT_SLOTS; i++) {
		if(assets.txt.fonts[i] != NULL)
			TTF_CloseFont(assets.txt.fonts[i]);
	}

	/* Close the texture-table */
	for(i = 0; i < TEX_SLOTS; i++) {
		if(assets.tex.mask[i] == 0)
			continue;
		
		glDeleteTextures(1, &assets.tex.hdl[i]);
	}

	/* Close the shader-table */
	for(i = 0; i < SHD_SLOTS; i++) {
		if(assets.shd.mask[i] == 0)
			continue;
	
		glDeleteProgram(assets.shd.prog[i]);	
	}
}


static short shd_get_slot(void)
{
	short i;

	for(i = 0; i < SHD_SLOTS; i++) {
		if(assets.shd.mask[i] == 0)
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


extern short shd_set(char *name, char *vs, char *fs, int num, char **vars)
{
	int i;
	short slot;
	int success;
	char infoLog[512];
	uint32_t fshd = 0;
	uint32_t vshd = 0;
	char *vtx_src = NULL;
	char *frg_src = NULL;

	if((slot = shd_get_slot()) < 0) {
		ERR_LOG(("Shader-table already full"));
		return -1;
	}

	if(!vs || !fs)
		return -1;

	if((assets.shd.prog[slot] = glCreateProgram()) == 0) {
		ERR_LOG(("Failed to create shader-program"));
		goto err_cleanup;
	}

	/* Load vertex-shader and attach the vertex-shader */
	if(fs_load_file(vs, (uint8_t **)&vtx_src, NULL) < 0) {
		ERR_LOG(("Failed to load vtx-shader: %s", vs));
		goto err_cleanup;
	}

	/* Create and initialize the vertex-shader */
	vshd = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshd, 1, (const GLchar **)&vtx_src, NULL);
	glCompileShader(vshd);

	glGetShaderiv(vshd, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vshd, 512, NULL, infoLog);
		ERR_LOG(("Failed to compile shader"));
		printf("  %s: %s", vs, infoLog);
		goto err_cleanup;
	}

	glAttachShader(assets.shd.prog[slot], vshd);

	/* Load and attach the fragment-shader */
	if(fs_load_file(fs, (uint8_t **)&frg_src, NULL) < 0) {
		ERR_LOG(("Failed to load frg-shader: %s", fs));
		goto err_cleanup;
	}

	/* Create and initialize the fragment-shader */
	fshd = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshd, 1, (const GLchar **)&frg_src, NULL);
	glCompileShader(fshd);

	glGetShaderiv(fshd, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fshd, 512, NULL, infoLog);
		ERR_LOG(("Failed to compile shader"));
		printf("  %s: %s", fs, infoLog);
		goto err_cleanup;
	}

	glAttachShader(assets.shd.prog[slot], fshd);

	/* Bind the vertex-attributes */
	for(i = 0; i < num; i++)
		glBindAttribLocation(assets.shd.prog[slot], i, vars[i]);

	/* Link the shader-program */
	glLinkProgram(assets.shd.prog[slot]);

	/* Detach and destroy assets.shd */
	glDetachShader(assets.shd.prog[slot], vshd);
	glDeleteShader(vshd);
	vshd = 0;

	glDetachShader(assets.shd.prog[slot], fshd);
	glDeleteShader(fshd);
	fshd = 0;

	assets.shd.mask[slot] = 1;
	strcpy(assets.shd.name[slot], name);
	free(vtx_src);
	free(frg_src);
	return slot;

err_cleanup:
	if(vshd) {
		glDetachShader(assets.shd.prog[slot], vshd);
		glDeleteShader(vshd);
	}
	if(fshd) {
		glDetachShader(assets.shd.prog[slot], fshd);
		glDeleteShader(fshd);
	}

	if(assets.shd.prog[slot])
		glDeleteProgram(assets.shd.prog[slot]);

	free(vtx_src);
	free(frg_src);
	return -1;
}


extern void shd_del(short slot)
{
	if(shd_check_slot(slot))
		return;

	if(assets.shd.mask[slot] == 0)
		return;
	
	glDeleteProgram(assets.shd.prog[slot]);
	assets.shd.mask[slot] = 0;
}


extern short shd_get(char *name)
{
	int i;

	for(i = 0; i < SHD_SLOTS; i++) {
		if(assets.shd.mask[i] == 0)
			continue;

		if(!strcmp(assets.shd.name[i], name))
			return i;
	}

	return -1;
}


extern void shd_use(short slot, int num, char **vars, int *loc)
{
	int i;

	if(shd_check_slot(slot))
		return;

	glUseProgram(assets.shd.prog[slot]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	/* Get the uniform-locations for the given variables */
	for(i = 0; i < num; i++)
		loc[i] = glGetUniformLocation(assets.shd.prog[slot], vars[i]);
}


extern void shd_unuse(void)
{
	glUseProgram(0);
}



static short tex_get_slot(void)
{
	short i;

	for(i = 0; i < TEX_SLOTS; i++) {
		if(assets.tex.mask[i] == 0)
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


extern short tex_set(char *name, uint8_t *px, int w, int h)
{
	short slot;

	if((slot = tex_get_slot()) < 0) {
		ERR_LOG(("Texture-table already full"));
		return -1;
	}

	glGenTextures(1, &assets.tex.hdl[slot]);
	glBindTexture(GL_TEXTURE_2D, assets.tex.hdl[slot]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, 
			GL_UNSIGNED_BYTE, px);

	glBindTexture(GL_TEXTURE_2D, 0);

	assets.tex.mask[slot] = 1;
	strcpy(assets.tex.name[slot], name);
	return slot;
}


extern short tex_load_png(char *name, char *pth)
{
	int w, h, ret = 0;
	uint8_t *px;

	if(fs_load_png(pth, &px, &w, &h) < 0) {
		ERR_LOG(("Failed to load texture: %s", pth));
		return -1;
	}

	if(tex_set(name, px, w, h) < 0) {
		ERR_LOG(("Failed add texture to table"));
		ret = -1;
	}

	free(px);
	return ret;
}


extern void tex_del(short slot)
{
	if(tex_check_slot(slot))
		return;

	if(assets.tex.mask[slot] == 0)
		return;

	glDeleteTextures(1, &assets.tex.hdl[slot]);
	assets.tex.mask[slot] = 0;
}


extern short tex_get(char *name)
{
	int i;

	for(i = 0; i < TEX_SLOTS; i++) {
		if(assets.tex.mask[i] == 0)
			continue;

		if(!strcmp(assets.tex.name[i], name))
			return i;
	}

	return -1;
}


extern void tex_use(short slot)
{
	if(tex_check_slot(slot))
		return;

	glBindTexture(GL_TEXTURE_2D, assets.tex.hdl[slot]);
}


extern void tex_unuse(void)
{
	glBindTexture(GL_TEXTURE_2D, 0);
}



extern short txt_load_ttf(char *pth, int size)
{
	font_t *font;

	if(assets.txt.font_num >= (TXT_FONT_SLOTS - 1)) {
		ERR_LOG(("Font-table is already full"));
		return -1;
	}

	if(!(font = TTF_OpenFont(pth, size))) {
		ERR_LOG(("Failed to load font %s", pth));
		return -1;
	}

	assets.txt.fonts[assets.txt.font_num] = font;
	assets.txt.font_num++;
	return assets.txt.font_num - 1;
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

	TTF_SetFontKerning(assets.txt.fonts[font], 0);
	TTF_SetFontHinting(assets.txt.fonts[font], TTF_HINTING_NORMAL);

	if(!(rend = TTF_RenderUTF8_Blended(assets.txt.fonts[font], text, *col)))
		return;

	reloff = u8_offset(text, rel);
	memcpy(subs, text, reloff);
	subs[reloff] = '\0';
	TTF_SizeUTF8(assets.txt.fonts[font], subs, &relw, &relh);

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

	if(!(f_ptr = assets.txt.fonts[font]))
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
