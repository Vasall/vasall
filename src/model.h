#ifndef _V_MODEL_H
#define _V_MODEL_H

#include "defines.h"
#include "vec.h"
#include "mat.h"
#include "texture.h"
#include "shader.h"
#include "utils.h"

#include <stdint.h>

#define MDL_NAME_MAX            8
#define MDL_SLOTS             256

#define MDL_OK                  0
#define MDL_ERR_CREATING        1
#define MDL_ERR_MESH            2
#define MDL_ERR_TEXTURE         3
#define MDL_ERR_LOADING         4
#define MDL_ERR_SHADER          5
#define MDL_ERR_FINISHING       6

/*
 * A struct used to store data about a single
 * model, which will then be pushed into the
 * model-cache, to enable crossreferncing from
 * multible objects at the same time.
 */
struct model {
	/*
	 * The name of the model.
	 */
	char name[9];

	/*
	 * The vertex-array-object
	 * attached to the model.
	 */
	uint32_t vao;

	/*
	 * The index-bao, index-buffer and
	 * number of indices in the buffer.
	 */
	uint32_t idx_bao;
	int idx_num;
	int *idx_buf;

	/*
	 * The number of vertices attached
	 * to this model.
	 */
	uint32_t vtx_bao;
	int vtx_num;
	float *vtx_buf;

	/*
	 * The texture attached to this
	 * model.
	 */
	short tex;

	/*
	 * The shader attached to this
	 * model.
	 */
	short shd;

	/*
	 * The current status of the model.
	 */
	uint8_t status;
};


/* The global model-table used to store all models */
V_GLOBAL struct model **models;

/*
 * Initialize the model-table.
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
V_API int mdl_init(void);

/*
 * Close the model-table and free the allocated memory. Also detach everything
 * from OpenGL, which has been used by the models.
 */
V_API void mdl_close(void);

/* 
 * Create a new model and allocate the necessary memory.
 *
 * @name: A null-terminated buffer with the max length of 8 letters containing
 * 	the name of the model
 *
 * Returns: Either the slot in the model-table or -1 if an error occurred
 */
V_API short mdl_set(char *name);

/* 
 * Attach a mesh to the model by copying the necessary data into the different 
 * buffers and adding them in OpenGL to the model.
 *
 * @slot: The slot in the model-table
 * @idxnum: The length of the index-buffer
 * @idx: A buffer containing the indices
 * @vtxnum: The number of vertices
 * @vtx: The vertex-positions
 * @nrm: The normal-vectors
 * @col: A buffer containing either uv-coordinates or color-values
 * @col_flg: A flag indicating if the col-buf contains colors or uv-coords
 */
V_API void mdl_set_mesh(short slot, int idxnum, int *idx, int vtxnum, 
		vec3_t *vtx, vec3_t *nrm, void *col, uint8_t col_flg);

/* 
 * Attach a texture to the model, by first copying the pixel-data into a
 * different buffer and then binding it using OpenGL.
 *
 * @slot: The slot in the model-table
 * @tex: The index of the texture in the texture-table
 */
V_API void mdl_set_texture(short slot, short tex);

/* 
 * Attach a shader from the shader-table to the model. This function will use
 * the given shader-key to get the shader from the shader-table and then add a
 * pointer to the struct to the model. If no shader with that key is in the
 * shader-table, the shader won't be set and the status of the model will be
 * updated.
 *
 * @slot: The slot in the model-table
 * @shd: The slot in the shader-table
 */
V_API void mdl_set_shader(short slot, short shd);

/* 
 * Load a new model from an dot-amo-file and attach both the texture and shader
 * to the model.
 *
 * @name: The name of the model
 * @amo: The absolute path the dot-amo-file containing the mesh
 * @tex: The index of the texture in the texture-table
 * @shd: The index of the shader in the shader-table
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
V_API short mdl_load(char *name, char *amo, short tex, short shd);

/*
 * Get a model from the model-table an return the pointer.
 *
 * @key: The name of the model to search for
 *
 * Returns: Either the slot in the model-table or -1 if an error occurred
 */
V_API short mdl_get(char *name);

/*
 * Remove a model from the model-table and free the allocated memory.
 *
 * @slot: The slot in the model-table
 */
V_API void mdl_del(short slot);

/*
 * Render a model with the given model-matrix.
 *
 * @slot: The slot in the model-table
 * @mat: The model-matrix to use
 */
V_API void mdl_render(short slot, mat4_t mat);

#endif
