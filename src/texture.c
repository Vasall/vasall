#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct texture_wrapper textures;


static short tex_get_slot(void)
{
	short i;

	for(i = 0; i < TEX_SLOTS; i++) {
		if(textures.mask[i] == 0)
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

extern int tex_init(void)
{
	int i;

	for(i = 0; i < TEX_SLOTS; i++)
		textures.mask[i] = 0;

	return 0;
}

extern void tex_close(void)
{
	int i;

	for(i = 0; i < TEX_SLOTS; i++) {
		if(textures.mask[i] == 0)
			continue;
		
		glDeleteTextures(1, &textures.handle[i]);
	}
}

extern short tex_set(char *name, uint8_t *px, int w, int h)
{
	short slot;

	if((slot = tex_get_slot()) < 0)
		return -1;

	glGenTextures(1, &textures.handle[slot]);
	glBindTexture(GL_TEXTURE_2D, textures.handle[slot]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, 
			GL_UNSIGNED_BYTE, px);

	glBindTexture(GL_TEXTURE_2D, 0);

	textures.mask[slot] = 1;
	strcpy(textures.name[slot], name);
	return slot;
}

extern void tex_del(short slot)
{
	if(tex_check_slot(slot))
		return;

	if(textures.mask[slot] == 0)
		return;

	glDeleteTextures(1, &textures.handle[slot]);
	textures.mask[slot] = 0;
}

extern short tex_get(char *name)
{
	int i;

	for(i = 0; i < TEX_SLOTS; i++) {
		if(textures.mask[i] == 0)
			continue;

		if(!strcmp(textures.name[i], name))
			return i;
	}

	return -1;
}

extern void tex_use(short slot)
{
	if(slot < 0)
		return;

	glBindTexture(GL_TEXTURE_2D, textures.handle[slot]);
}

extern void tex_unuse(void)
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

extern short tex_load_png(char *name, char *pth)
{
	int w, h, ret = 0;
	uint8_t *px;
	
	if(fs_load_png(pth, &px, &w, &h) < 0)
		return -1;

	if(tex_set(name, px, w, h) < 0)
		ret = -1;

	free(px);
	return ret;
}
