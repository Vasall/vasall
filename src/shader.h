#ifndef _SHADER_H_
#define _SHADER_H_

#include <stdint.h>


typedef struct Shader {
	/*
	 * The shader-program.
	 */
	uint32_t id;

	/*
	 * The vertex-shader.
	 */
	uint32_t vshdr;

	/*
	 * The fragment-shader.
	 */
	uint32_t fshdr;
} Shader;

/* Create a new shader program */
Shader *shdCreate(char *vtx_shd, char *frg_shd);

#endif
