#ifndef _MODEL_H_
#define _MODEL_H_

#include <stdint.h>
#include "vec.h"
#include "mat.h"
#include "shader.h"
#include "utils.h"

#define MESH_OK                  0
#define MESH_ERR_CREATING       -1
#define MESH_ERR_ADD_BAO        -2
#define MESH_ERR_SHADER         -3
#define MESH_ERR_FINISHING      -4

struct model {
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
	struct shader *shader;

	/*
	 * The status of the model.
	 */
	uint8_t status;
};

/* Start creating a new model and fill struct with default values */
struct model *mdlCreate(void);

/* Finish creating the new model */
int mdlFinish(struct model *mdl);

/* Set the vertices and indices of the model */
void mdlSetMesh(struct model *mdl, Vec3 *vtxbuf, int vtxlen, 
		uint32_t *idxbuf, int idxlen, uint8_t nrmflg);

/* Attach a new buffer to the model */
void mdlAddBAO(struct model *mdl, void *buf, int elsize, int num, 
		uint8_t bindflg, uint8_t bindsz);

/* Calculate the normal-vectors for the model */
void mdlCalcNormals(struct model *mdl);

/* Render a model */
void mdlRender(struct model *mdl, Mat4 mat);

/* Create a red-cube as a model (used for dev) */
struct model *mdlRedCube(void);

#endif
