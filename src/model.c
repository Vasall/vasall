#include "model.h"
#include "core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Redefine external variables */
struct model **models = NULL;

/*
 * Get an empty slot in the model-table.
 *
 * Returns: Either an empty slot or -1 if an error occurred
 */
V_INTERN short mdl_get_slot(void)
{
	short i;

	if(!models)
		return -1;

	for(i = 0; i < MDL_SLOTS; i++) {
		if(!models[i])
			return i;
	}

	return -1;
}

/*
 * Check if the slot-number is still in range.
 *
 * @slot: The slot-number to check
 *
 * Returns: Either 0 if the slot-number is ok, or -1 if not
 */
V_INTERN int mdl_check_slot(short slot)
{
	if(slot < 0 || slot >= MDL_SLOTS)
		return 1;

	return 0;
}

/*
 * Set the status of a model.
 *
 * @slot: The slot of the model in the model-table
 * @status: The new status of the model
 */
V_INTERN void mdl_set_status(short slot, uint8_t status)
{
	struct model *mdl;

	if(mdl_check_slot(slot))
		return;

	if(!(mdl = models[slot]))
		return;

	mdl->status = status;
}

V_API int mdl_init(void)
{
	int i;

	if(!(models = malloc(sizeof(struct model *) * MDL_SLOTS)))
		return -1;

	for(i = 0; i < MDL_SLOTS; i++)
		models[i] = NULL;

	return 0;
}

V_API void mdl_close(void)
{
	int i;
	struct model *mdl;

	for(i = 0; i < MDL_SLOTS; i++) {
		if(!(mdl = models[i]))
			continue;

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

		free(mdl);
	}

	free(models);
}

V_API short mdl_set(char *name)
{
	short slot;
	struct model *mdl;

	/* Check if the given key is valid */
	if(!name || strlen(name) >= MDL_NAME_MAX)
		return -1;

	if((slot = mdl_get_slot()) < 0)
		return -1;

	if(!(mdl = malloc(sizeof(struct model))))
		return -1;

	/* Copy the key for this model */
	strcpy(mdl->name, name);

	/* Initialize model-attributes */
	mdl->vao = 0;
	mdl->idx_bao = 0;
	mdl->idx_buf = NULL;
	mdl->idx_num = 0;	
	mdl->vtx_bao = 0;
	mdl->vtx_buf = NULL;
	mdl->vtx_num = 0;
	mdl->tex = -1;
	mdl->shd = -1;
		
	mdl->status = MDL_OK;

	/* Generate a new vao */
	glGenVertexArrays(1, &mdl->vao);

	models[slot] = mdl;
	return slot;
}

V_API void mdl_set_mesh(short slot, int idxnum, int *idx, int vtxnum,
		vec3_t *vtx, vec3_t *nrm, void *col, uint8_t col_flg)
{
	int i;
	float *ptr;
	int col_sizeb = (col_flg == 0) ? (3) : (2);
	int col_size = col_sizeb * sizeof(float);
	int vtx_sizeb = ((3 * 2) + col_sizeb);
	int vtx_size = vtx_sizeb * sizeof(float);
	struct model *mdl;

	if(mdl_check_slot(slot))
		return;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		goto err_set_failed;

	/* Allocate memory for the indices */
	mdl->idx_num = idxnum;
	if(!(mdl->idx_buf = malloc(idxnum * sizeof(int))))
		goto err_set_failed;

	/* Allocate memory for the vertex-data */
	mdl->vtx_num = vtxnum;
	if(!(mdl->vtx_buf = malloc(vtxnum * vtx_size)))
		goto err_set_failed;

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
	glBufferData(GL_ARRAY_BUFFER, vtxnum * vtx_size, mdl->vtx_buf, 
			GL_STATIC_DRAW);

	/* Bind the data to the indices */
	/* Vertex-Position */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vtx_size, NULL);
	/* Normal-Vector */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vtx_size, 
			(void *)(3 * sizeof(float)));
	/* UV-Coordinate */
	glVertexAttribPointer(2, col_sizeb, GL_FLOAT, GL_FALSE, vtx_size, 
			(void *)(6 * sizeof(float)));
	
	/* Register the indices */
	glGenBuffers(1, &mdl->idx_bao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mdl->idx_bao);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxnum * sizeof(int), 
			mdl->idx_buf, GL_STATIC_DRAW);

	/* Unbind the vertex-array-object */
	glBindVertexArray(0);
	return;

err_set_failed:
	mdl_set_status(slot, MDL_ERR_MESH);
}

V_API void mdl_set_texture(short slot, short tex)
{
	struct model *mdl;

	if(mdl_check_slot(slot))
		return;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		goto err_set_failed;

	mdl->tex = tex;
	return;

err_set_failed:
	mdl_set_status(slot, MDL_ERR_TEXTURE);
}

V_API void mdl_set_shader(short slot, short shd)
{
	struct model *mdl;
	
	if(mdl_check_slot(slot))
		return;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		goto err_set_failed;

	mdl->shd = shd;
	return;

err_set_failed:
	mdl_set_status(slot, MDL_ERR_SHADER);
}

V_API short mdl_load(char *name, char *amo, short tex, short shd)
{
	int slot;
	int *idx;
	int idxnum;
	int vtxnum;
	vec3_t *vtx;
	vec3_t *nrm;
	vec2_t *uv;

	/* Allocate memory for the model-struct */
	if((slot = mdl_set(name)) < 0)
		return -1;

	if(mdl_load_obj(amo, &idxnum, &idx, &vtxnum, &vtx, &nrm, &uv) < 0)
		goto err_del_mdl;

	mdl_set_texture(slot, tex);
	mdl_set_shader(slot, shd);

	mdl_set_mesh(slot, idxnum, idx, vtxnum, vtx, nrm, uv, 1);

	mdl_set_status(slot, MDL_OK);
	return slot;

err_del_mdl:
	mdl_del(slot);
	return -1;
}

V_API short mdl_get(char *name)
{
	int i;

	for(i = 0; i < MDL_SLOTS; i++) {
		if(!models[i])
			continue;

		if(!strcmp(models[i]->name, name)) {
			return i;
		}
	}

	return -1;
}

V_API void mdl_del(short slot)
{
	struct model *mdl;

	if(mdl_check_slot(slot))
		return;

	if(!(mdl = models[slot]))
		return;

	if(mdl->idx_bao)
		glDeleteBuffers(1, &mdl->idx_bao);
	
	if(mdl->vtx_bao)
		glDeleteBuffers(1, &mdl->vtx_bao);

	if(mdl->vao)
		glDeleteVertexArrays(1, &mdl->vao);

	if(mdl->idx_buf)
		free(mdl->idx_buf);
	
	if(mdl->vtx_buf)
		free(mdl->vtx_buf);

	free(mdl);
	models[slot] = NULL;
}

V_API void mdl_render(short slot, mat4_t mat)
{
	int model, view, proj;
	mat4_t mod, vie, pro;
	struct model *mdl;
	struct shader *shd;
	struct texture *tex;

	if(mdl_check_slot(slot))
		return;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		return;

	if(!(shd = shaders[mdl->shd]))
		return;

	if(!(tex = textures[mdl->tex]))
		return;
	
	mat4_cpy(mod, mat);
	cam_get_view(vie);
	cam_get_proj(pro);

	glBindVertexArray(mdl->vao);
	glUseProgram(shd->prog);

	if(mdl->tex >= 0)
		glBindTexture(GL_TEXTURE_2D, tex->id);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	model = glGetUniformLocation(shd->prog, "model");
	view = glGetUniformLocation(shd->prog, "view");
	proj = glGetUniformLocation(shd->prog, "proj");

	glUniformMatrix4fv(model, 1, GL_FALSE, mod);
	glUniformMatrix4fv(view, 1, GL_FALSE, vie);
	glUniformMatrix4fv(proj, 1, GL_FALSE, pro);

	glDrawElements(GL_TRIANGLES, mdl->idx_num, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
	glBindVertexArray(0);
}
