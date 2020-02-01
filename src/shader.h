#ifndef _SHADER_H_
#define _SHADER_H_

#include <stdint.h>

#define SHD_SLOTS               8

#define SHADER_ERR_LOAD		1
#define SHADER_ERR_COMPILE	2
#define SHADER_ERR_LINK		4

struct shader {	/*
	 * The shader-program.
	 */
	uint32_t prog;

	/*
	 * The vertex-shader.
	 */
	uint32_t vshdr;

	/*
	 * The fragment-shader.
	 */
	uint32_t fshdr;

	/*
	 * The status of the shader.
	 */
	uint8_t status;
};


/* A list containing all active shaders */
extern struct ht_t *shader_table;


/* Initialize the shader-table */
int shdInit(void);

/* Destroy the shader-table */
void shdClose(void);

/* Create a new shader and attach it to the shader-table */
int shdSet(char *key, char *vtx_shd, char *frg_shd);

/* Get a shader from the shader-table */
struct shader *shdGet(char *key);

/* Delete a shader an remove it from the shader-table */
void shdDel(char *key);

#endif
