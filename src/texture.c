#include "texture.h"
#include "core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Redefine external variables */
struct ht_t *textures = NULL;


int tex_init(void)
{
	if(!(textures = ht_init(TEX_SLOTS)))
		return -1;

	return 0;
}

void tex_close(void)
{
	int i;
	struct ht_entry *ptr;
	struct texture *tex;

	for(i = 0; i < textures->size; i++) {
		ptr = textures->entries[i];

		while(ptr != NULL) {
			tex = (struct texture *)ptr->buf;

			glDeleteTextures(1, &tex->id);
			free(tex->buf);

			ptr = ptr->next;
		}
	}

	ht_close(textures);
}

int tex_set(char *key, uint8_t *buf, int w, int h)
{
	struct texture *tex;
	int tex_sz = sizeof(struct texture);
	int size = (w * h) * sizeof(uint8_t) * 4;
		
	if(!(tex = malloc(tex_sz)))
		return -1;

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

	/* Add the texture to the table */
	if(ht_set(textures, key, (const uint8_t *)tex, tex_sz) < 0)
		goto err_free_tex;

	/* Delete this instance, as it has been copied into the table */
	free(tex);
	return 0;

err_free_tex:
	free(tex);
	return -1;
}

int tex_load(char *key, char *pth)
{
	int w, h;
	uint8_t *buf;

	/* Load the PNG-file */
	texLoadPNG(pth, &buf, &w, &h);

	return tex_set(key, buf, w, h);
}

struct texture *tex_get(char *key)
{
	struct texture *tex;

	if(ht_get(textures, key, (uint8_t **)&tex, NULL) < 0)
		return NULL;

	return tex;
}

void tex_del(char *key)
{
	if(key) {}
}
