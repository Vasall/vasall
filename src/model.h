#ifndef _MODEL_H
#define _MODEL_H

#include <stdint.h>
#include "vec.h"
#include "mat.h"
#include "texture.h"
#include "shader.h"
#include "utils.h"

#define MDL_KEY_LEN             5
#define MDL_SLOTS               8

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
	 * The key of this model.
	 */
	char key[8];
	
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
	struct texture *tex;

	/*
	 * The shader attached to this
	 * model.
	 */
	struct shader *shader;

	/*
	 * The current status of the model.
	 */
	uint8_t status;
};


/* The global model-table used to store all models */
extern struct ht_t *models;

/*
 * Initialize the model-cache.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int mdl_init(void);

/*
 * Close the model-cache and free the
 * allocated memory. Also detach everything
 * from OpenGL, which has been used by the
 * models.
 */
void mdl_close(void);

/* 
 * Create a new model by allocating the
 * necessary memory. Note that this function
 * doesn't yet push the model into the
 * model-table, as this will be done when
 * finishing the model.
 *
 * @key: The key to bind the model to
 *
 * Returns: Either a pointer to the model
 * 	or NULL if an error occurred
 */
struct model *mdl_begin(char *key);

/* 
 * Attach a mesh to the model by copying the
 * necessary data into the different buffers
 * and adding them in OpenGL to the model.
 *
 * @mdl: Pointer to the model
 * @idxnum: The length of the index-buffer
 * @idx: A buffer containing the indices
 * @vtxnum: The number of vertices
 * @vtx: The vertex-positions
 * @nrm: The normal-vectors
 * @col: A buffer containing either uv-coordinates or color-values
 * @col_flg: A flag indicating if the col-buf contains colors or uv-coords
 */
void mdl_set_mesh(struct model *mdl, int idxnum, int *idx, 
		int vtxnum, vec3_t *vtx, vec3_t *nrm, void *col,
		uint8_t col_flg);

/* 
 * Attach a texture to the model, by first
 * copying the pixel-data into a different
 * buffer and then binding it using OpenGL.
 *
 * @mdl: Pointer to the model
 * @tex: The key of the texture to use
 */
void mdl_set_texture(struct model *mdl, char *tex);

/* 
 * Attach a shader from the shader-table 
 * to the model. This function will use
 * the given shader-key to get the shader
 * from the shader-table and then add a
 * pointer to the struct to the model.
 * If no shader with that key is in the
 * shader-table, the shader won't be set
 * and the status of the model will be
 * updated.
 *
 * @mdl: Pointer to the model
 * @shd: The key for the shader in the shader-table
 */
void mdl_set_shader(struct model *mdl, char *shd);

/* 
 * Finish the model and push it into the model-table.
 * Note that this function will only put the model
 * into the table, if the status is MDL_OK, which
 * means that the model has been created successfully.
 * To get the actual status of the model, just check
 * the status-attribute.
 *
 * @mdl: Pointer to the model to finish
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int mdl_end(struct model *mdl);

/* 
 * Start the creation of a new model. 
 * First push a new model struct into 
 * the model-cache and mark the new 
 * struct as the currently active one.
 *
 * Returns: Either a pointer to the new
 * 	model of NULL
 */
int mdl_load(char *key, char *obj, char *tex, char *shd);

/*
 * Get a model from the model-table an return the pointer.
 *
 * @key: The key of the model
 *
 * Returns: Either the pointer to the model or NULL
 * 	if an error occurred
 */
struct model *mdl_get(char *key);

/*
 * This function is going to be used, to
 * delete a standalone model, which hasn't
 * been added to the model-table yet.
 *
 * @mdl: The pointer to the model
 */
void mdl_del(struct model *mdl);

/* 
 * Unbind everything from OpenGL, delete a model and 
 * remove it from the model-table.
 *
 * @key: The key of the model
 */
void mdl_remv(char *key);

/*
 * Render a model with the given model-matrix.
 *
 * @mdl: Pointer to the model to render
 * @mat: The model-matrix to use
 */
void mdl_render(struct model *mdl, mat4_t mat);

#endif
