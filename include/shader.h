#ifndef _SHADER_H
#define _SHADER_H

#include "sdl.h"

#include <stdint.h>

#define SHD_NAME_MAX            8
#define SHD_SLOTS               8

struct shader_wrapper {
	uint8_t mask[SHD_SLOTS];
	char name[SHD_SLOTS][9];
	uint32_t prog[SHD_SLOTS];
};


/* Define the global shader-wrapper instance */
extern struct shader_wrapper shaders;


/*
 * Initialize the shader-table.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int shd_init(void);


/*
 * Close the shader table, remove all shaders and free the allocated memory.
 */
extern void shd_close(void);


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

#endif
