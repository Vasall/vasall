#ifndef _SHADER_H_
#define _SHADER_H_

#include <stdint.h>

#define SHADER_ERR_LOAD		1
#define SHADER_ERR_COMPILE	2
#define SHADER_ERR_LINK		4

typedef struct Shader {
	/*
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
} Shader;

/* Create a new shader program */
Shader *shdCreate(char *vtx_shd, char *frg_shd);

/* Begin the creation of a new shader */
Shader *shdBegin(void);

/* Finish the creation of a new shader */
int shdFinish(Shader *shd);

/* Attach a vertex-shader to the program */
void shdAttachVtx(Shader *shd, char *pth);

/* Attach a fragment-shader to the program */
void shdAttachFrg(Shader *shd, char *pth);

/* Bind an attribute-location */
void shdBindAttrib(Shader *shd, int idx, char *name);

#endif
