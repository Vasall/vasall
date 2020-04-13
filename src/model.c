#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "model.h"
#include "global.h"

/* Redefine external variables */
struct ht_t *model_table = NULL;

/*
 * Initialize the model-cache.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int mdlInit(void)
{
	model_table = htCreate(MDL_SLOTS);
	if(model_table == NULL) return(-1);

	return(0);
}

/*
 * Close the model-cache and free the
 * allocated memory. Also detach everything
 * from OpenGL, which has been used by the
 * models.
 */
void mdlClose(void)
{
	int i;
	struct ht_entry *ptr;
	struct model *mdl;

	for(i = 0; i < model_table->size; i++) {
		ptr = model_table->entries[i];

		while(ptr != NULL) {
			mdl = (struct model *)ptr->buf;	

			/* Destroy buffer-array-objects */
			if(mdl->vtx_bao != 0) {
				glDeleteBuffers(1, &mdl->vtx_bao);
			}

			if(mdl->idx_bao != 0) {
				glDeleteBuffers(1, &mdl->idx_bao);	
			}

			/* Destroy vertex-array-object */
			if(mdl->vao != 0) {
				glDeleteVertexArrays(1, &mdl->vao);
			}

			/* Free buffers */
			if(mdl->idx_buf != NULL) free(mdl->idx_buf);
			if(mdl->vtx_buf != NULL) free(mdl->vtx_buf);

			ptr = ptr->next;
		}	
	}

	/* Destroy the model-table */
	htDestroy(model_table);
}

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
struct model *mdlCreate(char *key)
{
	struct model *mdl;

	/* Check if the given key is valid */
	if(key == NULL || strlen(key) >= MDL_KEY_LEN) return(NULL);

	mdl = malloc(sizeof(struct model));
	if(mdl == NULL) return(NULL);

	/* Copy the key for this model */
	strcpy(mdl->key, key);

	/* Initialize model-attributes */
	mdl->vao = 0;
	mdl->idx_bao = 0;
	mdl->idx_buf = NULL;
	mdl->idx_num = 0;	
	mdl->vtx_bao = 0;
	mdl->vtx_buf = NULL;
	mdl->vtx_num = 0;
	mdl->tex = NULL;
	mdl->shader = NULL;
		
	mdl->status = MDL_OK;

	/* Generate a new vao */
	glGenVertexArrays(1, &mdl->vao);

	return(mdl);
}

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
void mdlSetMesh(struct model *mdl, int idxnum, int *idx, 
		int vtxnum, vec3_t *vtx, vec3_t *nrm, void *col, 
		uint8_t col_flg)
{
	int i;
	float *ptr;
	int col_sizeb = (col_flg == 0) ? (3) : (2);
	int col_size = col_sizeb * sizeof(float);
	int vtx_sizeb = ((3 * 2) + col_sizeb);
	int vtx_size = vtx_sizeb * sizeof(float);

	if(mdl == NULL || mdl->status != MDL_OK) return;

	/* Allocate memory for the indices */
	mdl->idx_num = idxnum;
	mdl->idx_buf = malloc(idxnum * sizeof(int));
	if(mdl->idx_buf == NULL) goto failed;

	/* Allocate memory for the vertex-data */
	mdl->vtx_num = vtxnum;
	mdl->vtx_buf = malloc(vtxnum * vtx_size);
	if(mdl->vtx_buf == NULL) goto failed;

	/* Copy the indices into the allocated index-buffer */
	memcpy(mdl->idx_buf, idx, idxnum * sizeof(int));

	/* Create the vertex array and fill in the vertex-data */
	for(i = 0; i < vtxnum; i++) {
		ptr = mdl->vtx_buf + (i * vtx_sizeb);
		memcpy(ptr + 0, vtx[i], VEC3_SIZE);
		memcpy(ptr + 3, nrm[i], VEC3_SIZE);
		memcpy(ptr + 6, ((float *)col + i * col_sizeb), col_size);
	}

	/* Bind vertex-array-object */
	glBindVertexArray(mdl->vao);

	/* Register the vertex-positions */
	glGenBuffers(1, &mdl->vtx_bao);
	glBindBuffer(GL_ARRAY_BUFFER, mdl->vtx_bao);
	glBufferData(GL_ARRAY_BUFFER, vtxnum * vtx_size, 
			mdl->vtx_buf, GL_STATIC_DRAW);

	/* Bind the data to the indices */
	/* Vertex-Position */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
			vtx_size, NULL);
	/* Normal-Vector */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 
			vtx_size, (void *)(3 * sizeof(float)));
	/* UV-Coordinate */
	glVertexAttribPointer(2, col_sizeb, GL_FLOAT, GL_FALSE, 
			vtx_size, (void *)(6 * sizeof(float)));
	
	/* Register the indices */
	glGenBuffers(1, &mdl->idx_bao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mdl->idx_bao);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxnum * sizeof(int), mdl->idx_buf, GL_STATIC_DRAW);

	/* Unbind the vertex-array-object */
	glBindVertexArray(0);

	return;

failed:
	mdl->status = MDL_ERR_MESH;
}

/* 
 * Attach a texture to the model, by first
 * copying the pixel-data into a different
 * buffer and then binding it using OpenGL.
 *
 * @mdl: Pointer to the model
 * @tex: The key of the texture to use
 */
void mdlSetTex(struct model *mdl, char *tex)
{
	struct texture *texture;

	if(mdl == NULL || mdl->status != MDL_OK) return;

	texture = texGet(tex);
	if(texture == NULL) goto failed;

	mdl->tex = texture;
	return;

failed:
	mdl->status = MDL_ERR_TEXTURE;
}

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
void mdlSetShader(struct model *mdl, char *shd)
{
	struct shader *shd_ptr;
	
	if(mdl == NULL || mdl->status != MDL_OK) return;

	shd_ptr = shdGet(shd);
	if(shd_ptr == NULL) goto failed;

	mdl->shader = shd_ptr;
	return;

failed:
	mdl->status = MDL_ERR_SHADER;	
}

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
int mdlFinish(struct model *mdl)
{
	if(mdl == NULL || mdl->status != MDL_OK) return(-1);

	if(htSet(model_table, mdl->key, (const uint8_t *)mdl, 
			sizeof(struct model)) < 0) {
		goto failed;
	}

	/* Everything should have been copied into the table,
	 * so we should be able to free this struct. */
	free(mdl);

	return(0);

failed:
	mdl->status = MDL_ERR_FINISHING;
      	return(-1);
}

/* 
 * Start the creation of a new model. 
 * First push a new model struct into 
 * the model-cache and mark the new 
 * struct as the currently active one.
 *
 * Returns: Either a pointer to the new
 * 	model of NULL
 */
int mdlLoad(char *key, char *obj, char *tex, char *shd)
{
	struct model *mdl = NULL;
	int r, *idx, idxnum, vtxnum;
	vec3_t *vtx, *nrm;
	vec2_t *uv;

	/* Allocate memory for the model-struct */
	mdl = mdlCreate(key);
	if(mdl == NULL) goto failed;

	r = mdlLoadObj(obj, &idxnum, &idx, &vtxnum, &vtx, &nrm, &uv);
	if(r < 0) goto failed;

	mdlSetTex(mdl, tex);
	mdlSetShader(mdl, shd);

	mdlSetMesh(mdl, idxnum, idx, vtxnum, vtx, nrm, uv, 1);

	if(mdlFinish(mdl) < 0) goto failed;

	mdl->status = MDL_OK;

	return(0);

failed:
	/* Delete the local model-struct */
	mdlDel(mdl);

	return(-1);
}

/*
 * Get a model from the model-table an return the pointer.
 *
 * @key: The key of the model
 *
 * Returns: Either the pointer to the model or NULL
 * 	if an error occurred
 */
struct model *mdlGet(char *key)
{
	int r;
	struct model *mdl;

	r = htGet(model_table, key, (uint8_t **)&mdl, NULL);
	if(r < 0) return(NULL);

	return(mdl);
}

/*
 * This function is going to be used, to
 * delete a standalone model, which hasn't
 * been added to the model-table yet.
 *
 * @mdl: The pointer to the model
 */
void mdlDel(struct model *mdl)
{
	/* Destroy the buffer-array-objects */
	if(mdl->idx_bao != 0) glDeleteBuffers(1, &mdl->idx_bao);
	if(mdl->vtx_bao != 0) glDeleteBuffers(1, &mdl->vtx_bao);

	/* Destroy the vertex-array-object */
	if(mdl->vao != 0) glDeleteVertexArrays(1, &mdl->vao);

	/* Free the buffers */
	if(mdl->idx_buf != NULL) free(mdl->idx_buf);
	if(mdl->vtx_buf != NULL) free(mdl->vtx_buf);

	/* Free the struct itself */
	free(mdl);
}

/* 
 * Unbind everything from OpenGL, delete a model and 
 * remove it from the model-table.
 *
 * @key: The key of the model
 */
void mdlRemv(char *key)
{
	struct model *mdl;

	mdl = mdlGet(key);
	if(mdl == NULL) return;

	/* Destroy the buffer-array-objects */
	if(mdl->idx_bao != 0) glDeleteBuffers(1, &mdl->idx_bao);
	if(mdl->vtx_bao != 0) glDeleteBuffers(1, &mdl->vtx_bao);

	/* Destroy the vertex-array-object */
	if(mdl->vao != 0) glDeleteVertexArrays(1, &mdl->vao);

	/* Free the buffers */
	if(mdl->idx_buf != NULL) free(mdl->idx_buf);
	if(mdl->vtx_buf != NULL) free(mdl->vtx_buf);

	/* Remove the model from the model-table */
	htDel(model_table, mdl->key);
}

/*
 * Render a model with the given model-matrix.
 *
 * @mdl: Pointer to the model to render
 * @mat: The model-matrix to use
 */
void mdlRender(struct model *mdl, mat4_t mat)
{
	int model, view, proj;
	mat4_t mod, vie, pro;

	if(mdl == NULL || mdl->status != MDL_OK) return;

	mat4_cpy(mod, mat);
	camGetView(vie);
	camGetProj(pro);

	glBindVertexArray(mdl->vao);
	glUseProgram(mdl->shader->prog);

	if(mdl->tex != NULL) {
		glBindTexture(GL_TEXTURE_2D, mdl->tex->id);
	}

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	model = glGetUniformLocation(mdl->shader->prog, "model");
	view = glGetUniformLocation(mdl->shader->prog, "view");
	proj = glGetUniformLocation(mdl->shader->prog, "proj");

	glUniformMatrix4fv(model, 1, GL_FALSE, mod);
	glUniformMatrix4fv(view, 1, GL_FALSE, vie);
	glUniformMatrix4fv(proj, 1, GL_FALSE, pro);

	glDrawElements(GL_TRIANGLES, mdl->idx_num, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
	glBindVertexArray(0);
}
