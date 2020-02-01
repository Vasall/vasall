#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "XSDL/xsdl.h"

struct ht_t *tex_table = NULL;

/* 
 * Initialize the texture-table and
 * allocate the necessary memory.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int texInit(void)
{
	tex_table = htCreate(TEX_SLOTS);
	if(tex_table == NULL) return(-1);

	return(0);
}

/* 
 * Close the texture-table, delete all
 * textures and free the allocated memory.
 */
void texClose(void)
{

}

/* 
 * Create a new texture and push it
 * into the texture-table, with the
 * given key.
 *
 * @key: The key of the texture
 * @buf: A buffer containing the pixel-data
 * @w: The width of the texture
 * @h: The height of the texture
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int texSet(char *key, uint8_t *buf, int w, int h)
{
	struct texture *tex;
	int r, size = (w * h) * sizeof(uint8_t) * 3;

	tex = malloc(sizeof(struct texture));
	if(tex == NULL) return(-1);

	tex->width = w;
	tex->height = h;
	tex->size = w * h;

	tex->buf = malloc(size);
	if(tex->buf == NULL) {
		free(tex);
		return(-1);
	}

	memcpy(tex->buf, buf, size);

	glGenTextures(1, &tex->id);
	glBindTexture(GL_TEXTURE_2D, tex->id);

	/* Configure the texture */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	/* Transfer the pixel-data to OpenGL */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, 
			GL_UNSIGNED_BYTE, tex->buf);

	/* Unbind current texture */
	glBindTexture(GL_TEXTURE_2D, 0);

	/* Add the texture to the table */
	r = htSet(tex_table, key, (const uint8_t *)tex, sizeof(struct texture));
	if(r < 0) return(-1);

	return(0);
}

/* 
 * Load a texture from a file and
 * push the created structure into
 * the texture-table, with the given
 * key.
 *
 * @key: The key of the texture
 * @pth: The absolute path to the texture
 */
int texLoad(char *key, char *pth)
{
	int i;
	uint8_t *buf;

	if(pth) {}

	buf = malloc(32 * 32 * 3 * sizeof(uint8_t));
	if(buf == NULL) return(-1);

	for(i = 0; i < 32 * 32 * 3 * sizeof(uint8_t); i += 3) {
		buf[i] = 255;
		buf[i + 1] = 0;
		buf[i + 2] = 0;
	}

	return(texSet(key, buf, 32, 32));
}

/* 
 * Get a texture from the table and
 * return a pointer to the struct.
 *
 * @key: The key of the texture
 *
 * Returns: Either a pointer to the struct
 * 	or NULL if an error occurred
 */
struct texture *texGet(char *key)
{
	struct texture *tex;
	int r;
	
	r = htGet(tex_table, key, (uint8_t **)&tex, NULL);
	if(r < 0) return(NULL);

	return(tex);
}

/* 
 * Delete a texture, remove it from
 * the table and free the allocated memory.
 *
 * @key: The key of the texture
 */
void texDel(char *key)
{
	if(key) {}
}
