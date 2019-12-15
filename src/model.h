#ifndef _MODEL_H_
#define _MODEL_H_

#include <stdint.h>
#include "shader.h"
#include "stdutil.h"

typedef struct Model {
	uint32_t vao;
	uint32_t vbo;
	uint32_t ibo;
	uint32_t cbo;
	uint32_t nbo;
} Model;

/* Create a new model */
/*Model *modCreate(Vec3 pos, Vec3 size, float *vtx, int vnum, Color *col, 
		int cnum, Vertex *norm, int nnum, char *vtxshd, char *frgshd);*/

/* Render a model */
void modRender(Model *mod);

#endif
