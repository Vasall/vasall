#include "texture.h"
#include "core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Redefine external variables */
struct texture **textures = NULL;

static short tex_get_slot(void)
{
	short i;

	if(!textures)
		return -1;

	for(i = 0; i < TEX_SLOTS; i++) {
		if(!textures[i])
			return i;
	}

	return -1;
}

int tex_init(void)
{
	int i;
	short slot;

	if(!(textures = malloc(sizeof(struct texture *) * TEX_SLOTS)))
		return -1;

	for(i = 0; i < TEX_SLOTS; i++)
		textures[i] = NULL;

	return 0;
}

void tex_close(void)
{
	int i;
	struct texture *tex;

	for(i = 0; i < TEX_SLOTS; i++) {
		if(!(tex = textures[i]))
			continue;
		
		glDeleteTextures(1, &tex->id);
		free(tex->buf);
		free(tex);
	}

	free(textures);
}

short tex_set(char *name, uint8_t *buf, int w, int h)
{
	struct texture *tex;
	int tex_sz = sizeof(struct texture);
	int size = (w * h) * sizeof(uint8_t) * 4;
	short slot;

	if((slot = tex_get_slot()) < 0)
		return -1;

	if(!(tex = malloc(tex_sz)))
		return -1;

	strcpy(tex->name, name);
	tex->width = w;
	tex->height = h;
	tex->size = w * h;

	if(!(tex->buf = malloc(size)))
		goto err_free_tex;

	memcpy(tex->buf, buf, size);

	glGenTextures(1, &tex->id);
	glBindTexture(GL_TEXTURE_2D, tex->id);

	/* Configure the texture */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Transfer the pixel-data to OpenGL */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, 
			GL_UNSIGNED_BYTE, tex->buf);

	/* Unbind current texture */
	glBindTexture(GL_TEXTURE_2D, 0);

	textures[slot] = tex;
	return slot;

err_free_tex:
	free(tex);
	return -1;
}

short tex_load(char *name, char *pth)
{
	int w, h;
	uint8_t *buf;
	
	/* Load the PNG-file */
	texLoadPNG(pth, &buf, &w, &h);

	return tex_set(name, buf, w, h);
}

short tex_get(char *name)
{
	int i;

	for(i = 0; i < TEX_SLOTS; i++) {
		if(!textures[i])
			continue;

		if(!strcmp(textures[i]->name, name))
			return i;
	}

	return -1;
}

void tex_del(short slot)
{
	struct texture *tex;

	if(slot < 0 || slot >= TEX_SLOTS)
		return;

	if(!(tex = textures[slot]))
		return;

	glDeleteTextures(1, &tex->id);
	free(tex->buf);
	free(tex);
	textures[slot] = NULL;
}
