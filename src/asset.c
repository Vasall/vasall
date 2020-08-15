#include "asset.h"
#include "error.h"

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
	if(txt_init() < 0)
		return -1;

	return 0;
}


extern void ast_close(void)
{
	short i;

	/* Close the font-table */
	txt_close();

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


extern short shd_set(char *name, char *vs, char *fs)
{
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
	glBindAttribLocation(assets.shd.prog[slot], 0, "vtxPos");
	glBindAttribLocation(assets.shd.prog[slot], 1, "vtxNrm");
	glBindAttribLocation(assets.shd.prog[slot], 2, "vtxVal");

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


extern void shd_use(short slot, int *loc)
{
	if(shd_check_slot(slot))
		return;

	glUseProgram(assets.shd.prog[slot]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	loc[0] = glGetUniformLocation(assets.shd.prog[slot], "model");
	loc[1] = glGetUniformLocation(assets.shd.prog[slot], "view");
	loc[2] = glGetUniformLocation(assets.shd.prog[slot], "proj");
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
