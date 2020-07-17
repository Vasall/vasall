#ifndef _ASSET_H
#define _ASSET_H

#include "sdl.h"
#include "filesystem.h"
#include "text.h"

#include <stdint.h>


#define SHD_NAME_MAX            8
#define SHD_SLOTS               8

struct shader_wrapper {
	uint8_t    mask[SHD_SLOTS];
	char       name[SHD_SLOTS][SHD_NAME_MAX+1];
	uint32_t   prog[SHD_SLOTS];
};


#define TEX_NAME_MAX            8
#define TEX_SLOTS               8

struct texture_wrapper {
	uint8_t    mask[TEX_SLOTS];
	char       name[TEX_SLOTS][TEX_NAME_MAX+1];
	uint32_t   hdl[TEX_SLOTS];
};


struct asset_wrapper {
	struct shader_wrapper shd;
	struct texture_wrapper tex;
};


/* The global asset-wrapper */
extern struct asset_wrapper assets;


/*
 * This function works as a wrapper function to initialize all asset-tables,
 * ie shaders, textures and fonts. 
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int ast_init(void);


/*
 * Close the asset-tables and free the allocated memory.
 */
extern void ast_close(void);


/* ----------------------------------------------- */
/*                                                 */
/*                    SHADER                       */
/*                                                 */
/* ------------------------------------------------*/

/*
 * Create a new shader-entry and add it to the shader-table.
 *
 * @name: The name of the shader
 * @vs: A null-terminated buffer containing the vertex-shader
 * @fs: A null-terminated buffer containing the fragment-shader
 *
 * Returns: The slot of the shader in the table or -1 if an error occurred
 */
extern short shd_set(char *name, char *vs, char *fs);


/*
 * Remove a shader from the table and delete it.
 *
 * @slot: The slot of the shader in the table
 */
extern void shd_del(short slot);


/*
 * Get the slot of a shader in the table by searching for the name.
 *
 * @name: The name of the shader to search for
 *
 * Returns: Either the slot the shader is on or -1 if an error occurred
 */
extern short shd_get(char *name);


/*
 * Tell OpenGL to use a shader.
 *
 * @slot: The slot the shader is on
 * @loc: A buffer to write the indices of the uniform-vars to
 */
extern void shd_use(short slot, int *loc);


/* 
 * Tell OpenGL to unuse the current shader.
 */
extern void shd_unuse(void);


/* ----------------------------------------------- */
/*                                                 */
/*                   TEXTURE                       */
/*                                                 */
/* ------------------------------------------------*/

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
