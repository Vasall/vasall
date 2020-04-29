#ifndef _V_TEXTURE_H
#define _V_TEXTURE_H

#include "defines.h"
#include "core.h"
#include "vec.h"
#include "hashtable.h"
#include "tex_utils.h"

#include <stdint.h>

#define TEX_NAME_MAX            8
#define TEX_SLOTS               8

struct texture {
	/*
	 * The name of the texture.
	 */
	char name[9];

	uint32_t id;

	/* The buffer containing the texture */
	uint8_t *buf;

	/* The size of this texture */
	int width;
	int height;
	int size;
};

/* The global texture-table */
V_GLOBAL struct texture **textures;

/*
 * Initialize the texture-table and allocate the necessary memory.
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
V_API int tex_init(void);

/*
 * Close the texture-table, delete all textures and free the allocated memory.
 */
V_API void tex_close(void);

/*
 * Create a new texture and push it into the texture-table, with the 
 * given name.
 *
 * @name: The name of the texture
 * @buf: A buffer containing the pixel-data
 * @w: The width of the texture
 * @h: The height of the texture
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
V_API short tex_set(char *name, uint8_t *buf, int w, int h);

/*
 * Load a texture from a file and push the created structure into the 
 * texture-table, with the given key.
 *
 * @name: The name of the texture
 * @pth: The absolute path to the texture
 *
 * Returns: Either the slot in the texture-table or -1 if an error occurred
 */
V_API short tex_load(char *name, char *pth);

/*
 * Get a texture from the texture-table.
 *
 * @name: The name of the texture
 *
 * Returns: Either the slot in the texture-table or -1 if an error occurred
 */
V_API short tex_get(char *name);

/*
 * Delete a texture, remove it from the table and free the allocated memory.
 *
 * @slot: The slot in the texture-table
 */
V_API void tex_del(short slot);

#endif
