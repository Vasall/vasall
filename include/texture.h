#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "filesystem.h"
#include "sdl.h"

#include <stdint.h>

#define TEX_NAME_MAX            8
#define TEX_SLOTS               8

struct texture_wrapper {
	uint8_t mask[TEX_SLOTS];
	char name[TEX_SLOTS][9];
	uint32_t handle[TEX_SLOTS];
};


/* Define the global texture-wrapper */
extern struct texture_wrapper textures;


/*
 * Initialize a the texture-table.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int tex_init(void);


/*
 * CLose the texture-table, remove the loaded textures and free the allocated
 * memory.
 */
extern void tex_close(void);


/*
 * Manually set a new texture and add it to the texture-table.
 *
 * @name: The name of the texture
 * @px: A buffer containing the RGBA-pixels
 * @w: The with of the texture
 * @h: The height of the texture
 *
 * Returns: Either the slot the texture is on or -1 if an error occurred
 */
extern short tex_set(char *name, uint8_t *px, int w, int h);


/*
 * Delete a texture, remove it from the texture-table and free the allocated
 * memory.
 *
 * @slot: The slot of the texture to delete
 */
extern void tex_del(short slot);


/*
 * Get the slot a texture by searching for a texture-name.
 *
 * @name: the name to search for
 *
 * Returns: The slot the texture is on or -1 if an error occurred
 */
extern short tex_get(char *name);


/*
 * Tell OpenGL to use the texture.
 *
 * @slot: The slot the texture is on
 */
extern void tex_use(short slot);


/*
 * Tell OpenGL to stop using the current texture.
 */
extern void tex_unuse(void);


/*
 * Load a texture from a PNG-file and add it to the texture-table.
 *
 * @name: The name of the texture
 * @pth: The ralative path to the PNG-file containing the texture
 *
 * Returns: Either the slot the texture is on or -1 if an error occurred
 */
extern short tex_load_png(char *name, char *pth);

#endif
