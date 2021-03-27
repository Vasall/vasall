#ifndef _ASSET_H
#define _ASSET_H

#include "render_engine.h"
#include "sdl.h"
#include "filesystem.h"

#include <stdint.h>


#define SHD_NAME_MAX            8
#define SHD_SLOTS               8

struct shader_wrapper {
	uint8_t    mask[SHD_SLOTS];
	char       name[SHD_SLOTS][SHD_NAME_MAX+1];
	uint32_t   prog[SHD_SLOTS];
	struct vk_pipeline pipeline[SHD_SLOTS];
};


#define TEX_NAME_MAX            8
#define TEX_SLOTS               8

struct texture_wrapper {
	uint8_t    mask[TEX_SLOTS];
	char       name[TEX_SLOTS][TEX_NAME_MAX+1];
	uint32_t   hdl[TEX_SLOTS];
	struct vk_texture tex[TEX_SLOTS];
};


#define TXT_NAME_MAX            8
#define TXT_FONT_SLOTS          8

/*
 *  128  64  32  16   8   4   2   1
 *    7   6   5   4   3   2   1   0
 *
 * 0: Align text left
 * 1: Align text right
 * 2: UNDEFINED
 * 3: UNDEFINED
 * 4: Bold
 * 5: Italic
 * 6: Underline
 */

#define TXT_LEFT              0x01
#define TXT_RIGHT             0x02
#define TXT_BOLD              0x10
#define TXT_ITALIC            0x20
#define TXT_UNDERLINE         0x40

struct text_wrapper {
	short font_num;
	TTF_Font *fonts[TXT_FONT_SLOTS];
};


struct asset_wrapper {
	struct shader_wrapper shd;
	struct texture_wrapper tex;
	struct text_wrapper txt;
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
 * @num: The number of variables
 * @vars: An array of variable-names which will be bound to the corresponding
 *        position in the array
 *
 * Returns: The slot of the shader in the table or -1 if an error occurred
 */
extern short shd_set(char *name, char *vs, char *fs, int num, char **vars);


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
 * @attr: The upper limit to enable vertex-attributes up to
 */
extern void shd_use(short slot, int attr);


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
 * @pth: the path to png file
 *
 * Returns: Either the slot the texture is on or -1 if an error occurred
 */
extern short tex_set(char *name, char *pth);


/*
 * Manually set a new skybox texture and add it to the texture-table.
 *
 * @name: The name of the skybox texture
 * @pths: The paths of the skybox pngs (right, left, top, bottom, front, back)
 *
 * Returns: Either the slot the texture is on or -1 if an error occurred
 */
extern short skybox_set(char *name, char *pths[6]);


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


/* ----------------------------------------------- */
/*                                                 */
/*                     TEXT                        */
/*                                                 */
/* ------------------------------------------------*/

/*
 * Load a TTF-font and add it to the font-table.
 *
 * @pth: The relative path to the font
 * @size: The size to load the font at
 * 
 * Returns: Either the slot of the font in the table or -1 if an error occurred
 */
extern short txt_load_ttf(char *pth, int size);


/*
 * Render a text on a SDL-surface.
 *
 * @surf: The SDL-surface to render on
 * @rect: The rectangle to render the text in
 * @col: The color to render the text with
 * @font: The slot of the font in the table
 * @text: A null-terminated text-buffer
 * @rel: The relative offset to render the text with
 * @algn: The alignment of the text
 */
extern void txt_render_rel(surf_t *surf, rect_t *rect, color_t *col,
		short font, char *text, short rel, uint8_t algn);


/*
 * Render a text on a surface.
 *
 * @surf: The SDL-surface to render on
 * @rect: The rectangle to render the text in
 * @col: The color to render the text with
 * @font: The slot of the font in the table
 * @text: A null-terminated text-buffer
 * @opt: Additional options
 */
extern void txt_render(surf_t *surf, rect_t *rect, color_t *col,
		short font, char *text, uint8_t opt);


#endif
