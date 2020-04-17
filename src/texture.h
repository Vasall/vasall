#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <stdint.h>
#include "core.h"
#include "vec.h"
#include "hashtable.h"
#include "tex_utils.h"

#define TEX_SLOTS 8

struct texture {
	/* The id of this texture in OpenGL */
	uint32_t id;

	/* The buffer containing the texture */
	uint8_t *buf;

	/* The size of this texture */
	int width;
	int height;
	int size;
};

/* The global texture-table */
extern struct ht_t *textures;

/*
 * Initialize the texture-table and allocate the necessary memory.
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
int tex_init(void);

/*
 * Close the texture-table, delete all textures and free the allocated memory.
 */
void tex_close(void);

/*
 * Create a new texture and push it into the texture-table, with the 
 * given key.
 *
 * @key: The key of the texture
 * @buf: A buffer containing the pixel-data
 * @w: The width of the texture
 * @h: The height of the texture
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
int tex_set(char *key, uint8_t *buf, int w, int h);

/*
 * Load a texture from a file and push the created structure into the 
 * texture-table, with the given key.
 *
 * @key: The key of the texture
 * @pth: The absolute path to the texture
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
int tex_load(char *key, char *pth);

/*
 * Get a texture from the table and return a pointer to the struct.
 *
 * @key: The key of the texture
 *
 * Returns: Either a pointer to the texture or NULL if an error occurred
 */
struct texture *tex_get(char *key);

/*
 * Delete a texture, remove it from the table and free the allocated memory.
 *
 * @key: The key of the texture
 */
void tex_del(char *key);

#endif
