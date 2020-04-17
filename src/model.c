#include "model.h"
#include "core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Redefine external variables */
struct ht_t *models = NULL;

int mdl_init(void)
{
	if(!(models = ht_init(MDL_SLOTS)))
		return -1;

	return 0;
}

void mdl_close(void)
{
	int i;
	struct ht_entry *ptr;
	struct model *mdl;

	for(i = 0; i < models->size; i++) {
		ptr = models->entries[i];

		while(ptr != NULL) {
			mdl = (struct model *)ptr->buf;	

			/* Destroy buffer-array-objects */
			if(mdl->vtx_bao)
				glDeleteBuffers(1, &mdl->vtx_bao);

			if(mdl->idx_bao)
				glDeleteBuffers(1, &mdl->idx_bao);

			/* Destroy vertex-array-object */
			if(mdl->vao)
				glDeleteVertexArrays(1, &mdl->vao);

			/* Free buffers */
			if(mdl->idx_buf)
				free(mdl->idx_buf);

			if(mdl->vtx_buf)
				free(mdl->vtx_buf);

			ptr = ptr->next;
		}	
	}

	/* Destroy the model-table */
	ht_close(models);
}

struct model *mdl_begin(char *key)
{
	struct model *mdl;

	/* Check if the given key is valid */
	if(key == NULL || strlen(key) >= MDL_KEY_LEN)
		return NULL;

	if(!(mdl = malloc(sizeof(struct model))))
		return NULL;

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

	return mdl;
}

void mdl_set_mesh(struct model *mdl, int idxnum, int *idx, 
		int vtxnum, vec3_t *vtx, vec3_t *nrm, void *col, 
		uint8_t col_flg)
{
	int i;
	float *ptr;
	int col_sizeb = (col_flg == 0) ? (3) : (2);
	int col_size = col_sizeb * sizeof(float);
	int vtx_sizeb = ((3 * 2) + col_sizeb);
	int vtx_size = vtx_sizeb * sizeof(float);

	if(mdl == NULL || mdl->status != MDL_OK)
		return;

	/* Allocate memory for the indices */
	mdl->idx_num = idxnum;
	if(!(mdl->idx_buf = malloc(idxnum * sizeof(int))))
		goto err_set_status;

	/* Allocate memory for the vertex-data */
	mdl->vtx_num = vtxnum;
	if(!(mdl->vtx_buf = malloc(vtxnum * vtx_size)))
		goto err_set_status;

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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxnum * sizeof(int), 
			mdl->idx_buf, GL_STATIC_DRAW);

	/* Unbind the vertex-array-object */
	glBindVertexArray(0);

	return;

err_set_status:
	mdl->status = MDL_ERR_MESH;
}

void mdl_set_texture(struct model *mdl, char *tex)
{
	struct texture *tex_ptr = NULL;

	if(mdl == NULL || mdl->status != MDL_OK)
		return;

	if(!(tex_ptr = tex_get(tex)))
		goto err_set_status;

	mdl->tex = tex_ptr;
	return;

err_set_status:
	mdl->status = MDL_ERR_TEXTURE;
}

void mdl_set_shader(struct model *mdl, char *shd)
{
	struct shader *shd_ptr;
	
	if(mdl == NULL || mdl->status != MDL_OK)
		return;

	if(!(shd_ptr = shd_get(shd)))
		goto err_set_status;

	mdl->shader = shd_ptr;
	return;

err_set_status:
	mdl->status = MDL_ERR_SHADER;	
}

int mdl_end(struct model *mdl)
{
	int sz = sizeof(struct model);

	if(mdl == NULL || mdl->status != MDL_OK)
		return -1;

	if(ht_set(models, mdl->key, (const uint8_t *)mdl, sz) < 0)
		goto err_set_status;

	/* 
	 * Everything should have been copied into the table,
	 * so we should be able to free this struct.
	 */
	free(mdl);
	return 0;

err_set_status:
	mdl->status = MDL_ERR_FINISHING;
      	return -1;
}

int mdl_load(char *key, char *obj, char *tex, char *shd)
{
	struct model *mdl = NULL;
	int *idx, idxnum, vtxnum;
	vec3_t *vtx, *nrm;
	vec2_t *uv;

	/* Allocate memory for the model-struct */
	if(!(mdl = mdl_begin(key)))
		goto err_del_mdl;

	if(mdl_load_obj(obj, &idxnum, &idx, &vtxnum, &vtx, &nrm, &uv) < 0)
		goto err_del_mdl;

	mdl_set_texture(mdl, tex);
	mdl_set_shader(mdl, shd);

	mdl_set_mesh(mdl, idxnum, idx, vtxnum, vtx, nrm, uv, 1);

	if(mdl_end(mdl) < 0)
		goto err_del_mdl;

	mdl->status = MDL_OK;
	return 0;

err_del_mdl:
	mdl_del(mdl);
	return -1;
}

struct model *mdl_get(char *key)
{
	struct model *mdl;

	if(ht_get(models, key, (uint8_t **)&mdl, NULL) < 0)
		return NULL;

	return mdl;
}

void mdl_del(struct model *mdl)
{
	/* Destroy the buffer-array-objects */
	if(mdl->idx_bao)
		glDeleteBuffers(1, &mdl->idx_bao);
	
	if(mdl->vtx_bao)
		glDeleteBuffers(1, &mdl->vtx_bao);

	/* Destroy the vertex-array-object */
	if(mdl->vao)
		glDeleteVertexArrays(1, &mdl->vao);

	/* Free the buffers */
	if(mdl->idx_buf)
		free(mdl->idx_buf);
	
	if(mdl->vtx_buf)
		free(mdl->vtx_buf);

	/* Free the struct itself */
	free(mdl);
}

void mdl_remv(char *key)
{
	struct model *mdl;

	if(!(mdl = mdl_get(key)))
		return;

	/* Destroy the buffer-array-objects */
	if(mdl->idx_bao)
		glDeleteBuffers(1, &mdl->idx_bao);
	
	if(mdl->vtx_bao)
		glDeleteBuffers(1, &mdl->vtx_bao);

	/* Destroy the vertex-array-object */
	if(mdl->vao)
		glDeleteVertexArrays(1, &mdl->vao);

	/* Free the buffers */
	if(mdl->idx_buf)
		free(mdl->idx_buf);
	
	if(mdl->vtx_buf)
		free(mdl->vtx_buf);

	/* Remove the model from the model-table */
	ht_del(models, mdl->key);
}

void mdl_render(struct model *mdl, mat4_t mat)
{
	int model, view, proj;
	mat4_t mod, vie, pro;

	if(mdl == NULL || mdl->status != MDL_OK)
		return;

	mat4_cpy(mod, mat);
	camGetView(vie);
	camGetProj(pro);

	glBindVertexArray(mdl->vao);
	glUseProgram(mdl->shader->prog);

	if(mdl->tex)
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

	glUseProgram(0);
	glBindVertexArray(0);
}
