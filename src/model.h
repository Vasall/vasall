#ifndef _MODEL_H_
#define _MODEL_H_

#include <stdint.h>
#include "shader.h"
#include "stdutil.h"

typedef struct Model {
	/*
	 * The vertex-array-object
	 * attached to the model.
	 */
	uint32_t vao;

	/*
	 * The buffer-index-array, 
	 * containing the indices 
	 * of the buffers attached 
	 * to the vao.
	 */
	uint32_t bia[16];

	/*
	 * The number of buffers in
	 * the buffer-index-array.
	 */
	int8_t bia_num;

	/*
	 * The buffer-emable array,
	 * containing the buffers to
	 * enable for the vao.
	 */
	int8_t bea[16];

	/*
	 * The number of elements in
	 * the bea.
	 */
	int8_t bea_num;

	/* 
	 * The amount of vertices in
	 * the vertex-buffer-object.
	 */
	int vtx_len;

	/*
	 * The amount of indices in
	 * the index-buffer-object.
	 */
	int idx_len;

	/*
	 * The shader attached to this
	 * model.
	 */
	Shader *shader;

	/*
	 * The translation-matrix of
	 * the model.
	 */
	Mat4 trans;
		
	/*
	 * The rotation-matrix of the
	 * model.
	 */
	Mat4 rot;

	/*
	 * The scaling-matrix of the
	 * model.
	 */
	Mat4 scl;

	/*
	 * The status of the model.
	 */
	uint8_t status;
} Model;

/* Start creating a new model and fill struct with default values */
Model *mdlBegin(void);

/* Finish creating the new model */
int mdlEnd(Model *mdl);

/* Set the vertices and indices of the model */
void mdlLoadVtx(Model *mdl, Vertex *vtxbuf, int vtxlen, 
		uint32_t *idxbuf, int idxlen);

/* Attach colors to the model */
void mdlAttachColBuf(Model *mdl, ColorRGB *colbuf, int collen);

/* Attach a new buffer to the model */
void mdlAttachBuf(Model *mdl, void *buf, int elsize, int num, int8_t en);

/* Calculate the normal-vectors for the model */
void mdlCalcNormals(Model *mdl);

/* Render a model */
void mdlRender(Model *mdl);

/* Get the model-matrix for rendering */
Mat4 mdlGetMdlMat(Model *mdl);

/* Set the rotation of a model */
void mdlSetRot(Model *mdl, float x, float y, float z);

#endif
