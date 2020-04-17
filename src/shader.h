#ifndef _SHADER_H
#define _SHADER_H

#include "core.h"
#include <stdint.h>

#define SHD_SLOTS               8

#define SHADER_ERR_LOAD		1
#define SHADER_ERR_COMPILE	2
#define SHADER_ERR_LINK		4

struct shader {
	uint32_t prog;
	uint8_t status;
};


/* A list containing all active shaders */
extern struct ht_t *shaders;


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
 * Create a new shader and attach it to the shader-list.
 *
 * @key: The key of the shader
 * @vtx_shd: The absolute path to the vertex-shader
 * @frg_shd: The absolute path to the fragment-shader
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
int shd_set(char *key, char *vtx_shd, char *frg_shd);

/*
 * Get a shader from the shader-list by searching for a key.
 *
 * @key: The key of the shader
 *
 * Returns: Either a pointer to the shader or NULL if an error occurred
 */
struct shader *shd_get(char *key);

/*
 * Unbind everything from OpenGL, destroy the shader
 * and remove it from the shader-table.
 *
 * @key: The key of the shader to destroy
 */
void shd_remv(char *key);

#endif
