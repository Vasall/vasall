#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

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
 * allocated memory.
 */
void mdlClose(void)
{
	
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
	if(key == NULL || strlen(key) >= MDL_KEY_LEN) {
		return(NULL);
	}

	mdl = malloc(sizeof(struct model));
	if(mdl == NULL) return(NULL);


	/* Copy the key for this model */
	strcpy(mdl->key, key);
	
	/* Generate a new vao */
	glGenVertexArrays(1, &mdl->vao);

	mdl->idx_bao = -1;
	mdl->idx_buf = NULL;
	mdl->idx_num = 0;

	mdl->vtx_num = 0;
	
	mdl->vtx_bao = -1;
	mdl->vtx_buf = NULL;

	mdl->nrm_bao = -1;
	mdl->nrm_buf = NULL;

	mdl->uv_bao = -1;
	mdl->uv_buf = NULL;

	mdl->tex = NULL;

	mdl->shader = NULL;

	mdl->status = MDL_OK;

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
 * @uv: The uv-coordinates
 */
void mdlSetMesh(struct model *mdl, int idxnum, int *idx, 
		int vtxnum, Vec3 *vtx, Vec3 *nrm, Vec2 *uv)
{
	if(mdl == NULL || mdl->status != MDL_OK) return;

	mdl->idx_num = idxnum;

	mdl->idx_buf = malloc(idxnum * sizeof(int));
	if(mdl->idx_buf == NULL) goto failed;

	mdl->vtx_num = vtxnum;

	mdl->vtx_buf = malloc(vtxnum * VEC3_SIZE);
	if(mdl->vtx_buf == NULL) goto failed;

	mdl->nrm_buf = malloc(vtxnum * VEC3_SIZE);
	if(mdl->nrm_buf == NULL) goto failed;

	mdl->uv_buf = malloc(vtxnum * VEC2_SIZE);
	if(mdl->uv_buf == NULL) goto failed;

	/* Copy the different values into the dedicated buffers */
	memcpy(mdl->idx_buf, idx, idxnum * sizeof(int));
	memcpy(mdl->vtx_buf, vtx, vtxnum * VEC3_SIZE);
	memcpy(mdl->nrm_buf, nrm, vtxnum * VEC3_SIZE);
	memcpy(mdl->uv_buf, uv, vtxnum * VEC2_SIZE);

	/* Bind vertex-array-object */
	glBindVertexArray(mdl->vao);

	/* Register the vertex-positions */
	glGenBuffers(1, &mdl->vtx_bao);
	glBindBuffer(GL_ARRAY_BUFFER, mdl->vtx_bao);
	glBufferData(GL_ARRAY_BUFFER, vtxnum * VEC3_SIZE, mdl->vtx_buf, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	/* Register the indices */
	glGenBuffers(1, &mdl->idx_bao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mdl->idx_bao);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxnum * sizeof(int), mdl->idx_buf, GL_STATIC_DRAW);
	
	/* Register the normal-vectors */
	glGenBuffers(1, &mdl->nrm_bao);
	glBindBuffer(GL_ARRAY_BUFFER, mdl->nrm_bao);
	glBufferData(GL_ARRAY_BUFFER, vtxnum * VEC3_SIZE, mdl->nrm_buf, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	/* Register the uv-coorinates */
	glGenBuffers(1, &mdl->uv_bao);
	glBindBuffer(GL_ARRAY_BUFFER, mdl->uv_bao);
	glBufferData(GL_ARRAY_BUFFER, vtxnum * VEC2_SIZE, mdl->uv_buf, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

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
	int r;

	if(mdl == NULL || mdl->status != MDL_OK) return(-1);

	r = htSet(model_table, mdl->key, 
			(const uint8_t *)mdl, sizeof(struct model));
	if(r  < 0) goto failed;

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
	Vec3 *vtx, *nrm;
	Vec2 *uv;


	/* Allocate memory for the model-struct */
	mdl = mdlCreate(key);
	if(mdl == NULL) goto failed;

	r = mdlLoadObj(obj, &idxnum, &idx, &vtxnum, &vtx, &nrm, &uv);	
	if(r < 0) goto failed;

	mdlSetTex(mdl, tex);
	mdlSetShader(mdl, shd);

	mdlSetMesh(mdl, idxnum, idx, vtxnum, vtx, nrm, uv);

	printf("Finished %d : %d", mdlFinish(mdl), mdl->status);

	mdl->status = MDL_OK;

	return(0);

failed:
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
 * Delete a model and remove it from the model-table.
 * This function will also free the allocated memory.
 *
 * @key: The key of the model
 */
void mdlDel(char *key)
{
	if(key) {}
}

void mdlRender(struct model *mdl, Mat4 mat)
{
	int model, view, proj;
	Mat4 mod, vie, pro;

	if(mdl == NULL || mdl->status != MDL_OK) return;

	mat4Cpy(mod, mat);
	camGetView(vie);
	camGetProj(pro);

	glBindVertexArray(mdl->vao);
	glUseProgram(mdl->shader->prog);
	glBindTexture(GL_TEXTURE_2D, mdl->tex->id);

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

	glBindVertexArray(0);
}

/*
 * Loads a model from a wavefront .obj file.
 * The model gets added to the model cache and
 * the index is returned.
 *
 * @mdl: The model struct to set the mash of
 * @pth: The absolute path to the obj-file
 *
 * Returns: 0 on success, -1 on any error
 */
int mdlLoadObj(char *pth, int *idxnum, int **idx, int *vtxnum,
		Vec3 **vtx, Vec3 **nrm, Vec2 **uv)
{
	int ret = 0, i, j, tmp;
	FILE *fd;
	char char_buf[256];
	struct dyn_stack *vtx_in = NULL, *nrm_in = NULL, *tex_in = NULL;
	struct dyn_stack *vtx_out = NULL, *nrm_out = NULL, *tex_out = NULL;
	struct dyn_stack *idx_in = NULL, *idx_conv = NULL, *idx_out = NULL;

	/* Open the file in read-mode */
	if((fd = fopen(pth, "r")) == NULL) { 
		printf("test21\n");
		ret = -1; goto close; 
	}

	/* Create buffers to read input data into */
	if((vtx_in = stcCreate(VEC3_SIZE)) == NULL) goto failed;
	if((nrm_in = stcCreate(VEC3_SIZE)) == NULL) goto failed;
	if((tex_in = stcCreate(VEC2_SIZE)) == NULL) goto failed;

	/* Create buffers to write output data to */
	if((vtx_out = stcCreate(VEC3_SIZE)) == NULL) goto failed;
	if((nrm_out = stcCreate(VEC3_SIZE)) == NULL) goto failed;
	if((tex_out = stcCreate(VEC2_SIZE)) == NULL) goto failed;

	/* Create buffers to store the indices */
	if((idx_in = stcCreate(INT3)) == NULL) goto failed;
	if((idx_conv = stcCreate(INT3)) == NULL) goto failed;
	if((idx_out = stcCreate(sizeof(int))) == NULL) goto failed;

	/* Read the data from the obj-file */
	while(fscanf(fd, "%s", char_buf) != EOF) {
		if(strcmp(char_buf, "vt") == 0) {
			Vec2 tex_tmp;

			/* Read texture-data */
			fscanf(fd, "%f %f", 
					&tex_tmp[0], &tex_tmp[1]);

			stcPush(tex_in, &tex_tmp);

		}
		else if(strcmp(char_buf, "vn") == 0) {
			Vec3 nrm_tmp;

			/* Read the normal-vector */
			fscanf(fd, "%f %f %f", 
					&nrm_tmp[0], &nrm_tmp[1], &nrm_tmp[2]);

			/* Push the normal into the normal-stack */	
			stcPush(nrm_in, &nrm_tmp);
		}
		else if(strcmp(char_buf, "v") == 0) {
			Vec3 vtx_tmp;

			/* Read the vector-position */
			fscanf(fd, "%f %f %f", 
					&vtx_tmp[0], &vtx_tmp[1], &vtx_tmp[2]);

			/* Push the vertex into the vertex-array */
			stcPush(vtx_in, &vtx_tmp);
		}
		else if(strcmp(char_buf, "f") == 0) {
			int idx_tmp[3];

			/* Read the different indices */
			for(i = 0; i < 3; i++) {
				fscanf(fd, "%d/%d/%d", 
						&idx_tmp[0], &idx_tmp[1], &idx_tmp[2]);

				/* Obj-indices start at 1 */	
				for(j = 0; j < 3; j++) {
					idx_tmp[j] -= 1;
				}

				/* Push the indices into stack */
				stcPush(idx_in, idx_tmp);
			}
		}
	}

	/* Convert the data into a form usable for OpenGL */
	for(i = 0; i < idx_in->num; i++) {
		int cur[3], chk[3];
		uint8_t push = 1, same = 0;

		tmp = i;

		/* Copy the current indices into the cur-buffer */
		memcpy(cur, stcGet(idx_in, i), INT3);

		/* Check for similar vertices */
		for(j = 0; j < idx_conv->num; j++) {
			if(memcmp(cur, stcGet(idx_conv, j), INT3) == 0) {
				same = 1;

				tmp = j;
				stcPush(idx_out, &tmp);
				push = 0;

				break;
			}
		}

		if(!same) {
			for(j = 0; j < idx_conv->num; j++) {
				memcpy(chk, stcGet(idx_conv, j), INT3);

				/* If it's the same vertex */
				if(chk[0] == cur[0]) {
					/* Not the same normal-vector */
					if(chk[2] != cur[2] || 
							chk[2] != cur[2]) {
						/* Push the vertex to the end */
						tmp = stcPush(idx_conv, cur);

						/* Push the index into the index_list */
						stcPush(idx_out, &tmp);

						/* Prevent pushing again */
						push = 0;
						break;
					}
				}
			}
		}

		/* If pushing is enabled */
		if(push) {
			/* Push the indices into the list */
			tmp = stcPush(idx_conv, cur);

			/* Push the index into the index_list */
			stcPush(idx_out, &tmp);
		}
	}

	/* Push the data into the different arrays */
	for(i = 0; i < idx_conv->num; i++) {
		int cur[3];
		Vec3 vtx_tmp, nrm_tmp;
		Vec2 tex_tmp;

		memcpy(cur, stcGet(idx_conv, i), INT3);

		memcpy(vtx_tmp, stcGet(vtx_in, cur[0]), VEC3_SIZE);
		memcpy(tex_tmp, stcGet(tex_in, cur[1]), VEC2_SIZE);
		memcpy(nrm_tmp, stcGet(nrm_in, cur[2]), VEC3_SIZE);

		stcPush(vtx_out, vtx_tmp);
		stcPush(tex_out, tex_tmp);
		stcPush(nrm_out, nrm_tmp);
	}

	*idxnum = idx_out->num;

	*idx = malloc(*idxnum * sizeof(int));
	memcpy(*idx, idx_out->buf, idx_out->num * sizeof(int));

	*vtxnum = idx_conv->num;

	*vtx = malloc(*vtxnum * VEC3_SIZE);
	memcpy(*vtx, vtx_out->buf, vtx_out->num * VEC3_SIZE);
	
	*nrm = malloc(*vtxnum * VEC3_SIZE);
	memcpy(*nrm, nrm_out->buf, nrm_out->num * VEC3_SIZE);

	*uv = malloc(*vtxnum * VEC2_SIZE);
	memcpy(*uv, tex_out->buf, tex_out->num * VEC2_SIZE);

	goto cleanup;

failed:
	ret = -1;

cleanup:
	stcDestroy(vtx_in);
	stcDestroy(nrm_in);
	stcDestroy(tex_in);

	stcDestroy(vtx_out);
	stcDestroy(nrm_out);
	stcDestroy(tex_out);

	stcDestroy(idx_in);
	stcDestroy(idx_conv);
	stcDestroy(idx_out);

	fclose(fd);

close:
	return (ret);
}
