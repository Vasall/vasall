#include "texture.h"
#include "core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Redefine external variables */
struct texture **textures = NULL;

/*
 * Get an empty slot in the texture-table.
 *
 * Returns: Either an empty slot or -1 if an error occurred
 */
V_INTERN short tex_get_slot(void)
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

/*
 * Check if a slot-number is in range.
 *
 * @slot: The slot-number to check
 *
 * Returns: Either 0 if the slot-number is ok, or 1 if not 
 */
V_INTERN int tex_check_slot(short slot)
{
	if(slot < 0 || slot >= TEX_SLOTS)
		return 1;

	return 0;
}

V_API int tex_init(void)
{
	int i;

	if(!(textures = malloc(sizeof(struct texture *) * TEX_SLOTS)))
		return -1;

	for(i = 0; i < TEX_SLOTS; i++)
		textures[i] = NULL;

	return 0;
}

V_API void tex_close(void)
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

V_API short tex_set(char *name, uint8_t *buf, int w, int h)
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

V_API short tex_load(char *name, char *pth)
{
	int w, h;
	uint8_t *buf;
	
	if(tex_load_png(pth, &buf, &w, &h) < 0)
		return -1;

	return tex_set(name, buf, w, h);
}

V_API short tex_get(char *name)
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

V_API void tex_del(short slot)
{
	struct texture *tex;

	if(tex_check_slot(slot))
		return;

	if(!(tex = textures[slot]))
		return;

	glDeleteTextures(1, &tex->id);
	free(tex->buf);
	free(tex);
	textures[slot] = NULL;
}
