#ifndef _SHADER_H
#define _SHADER_H

#include "core.h"
#include <stdint.h>

#define SHD_SLOTS               8

#define SHADER_ERR_LOAD		1
#define SHADER_ERR_COMPILE	2
#define SHADER_ERR_LINK		4

struct shader {
	char name[9];
	uint32_t prog;
	uint8_t status;
};


/* A list containing all active shaders */
extern struct shader **shaders;

/*
 * Create and initialize the shader-list.
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
int shd_init(void);

/*
 * Close the shader-list and free the allocated memory.
 */
void shd_close(void);

/*
 * Create a new shader and attach it to the shader-table.
 *
 * @name: The name of the shader
 * @vtx_shd: The absolute path to the vertex-shader
 * @frg_shd: The absolute path to the fragment-shader
 *
 * Returns: Either a slot in the shader-table or -1 if an error occurred
 */
short shd_set(char *name, char *vtx_shd, char *frg_shd);

/*
 * Get a shader from the shader-list by searching for a name.
 *
 * @name: The name of the shader
 *
 * Returns: Either a slot in the shader-table or -1 if an error occurred
 */
short shd_get(char *name);

/*
 * Unbind everything from OpenGL, destroy the shader
 * and remove it from the shader-table.
 *
 * slot: The slot in the shader-table
 */
void shd_del(short slot);

#endif
