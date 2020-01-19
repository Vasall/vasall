#ifndef _SHADER_H_
#define _SHADER_H_

#include <stdint.h>

#define SHADER_ERR_LOAD		1
#define SHADER_ERR_COMPILE	2
#define SHADER_ERR_LINK		4

struct shader {
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
};

/* Create a new shader program */
struct shader *shdCreate(char *vtx_shd, char *frg_shd);

/* Begin the creation of a new shader */
struct shader *shdBegin(void);

/* Finish the creation of a new shader */
int shdFinish(struct shader *shd);

/* Attach a vertex-shader to the program */
void shdAttachVtx(struct shader *shd, char *pth);

/* Attach a fragment-shader to the program */
void shdAttachFrg(struct shader *shd, char *pth);

/* Bind an attribute-location */
void shdBindAttr(struct shader *shd, int idx, char *name);

#endif
