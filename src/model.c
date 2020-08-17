#include "model.h"
#include "error.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct model *models[MDL_SLOTS];


static short mdl_get_slot(void)
{
	short i;

	for(i = 0; i < MDL_SLOTS; i++) {
		if(!models[i])
			return i;
	}

	return -1;
}


static int mdl_check_slot(short slot)
{
	if(slot < 0 || slot >= MDL_SLOTS)
		return 1;

	return 0;
}


static void mdl_set_status(short slot, uint8_t status)
{
	struct model *mdl;

	if(mdl_check_slot(slot))
		return;

	if(!(mdl = models[slot]))
		return;

	mdl->status = status;
}


extern int mdl_init(void)
{
	int i;

	for(i = 0; i < MDL_SLOTS; i++)
		models[i] = NULL;

	return 0;
}


extern void mdl_close(void)
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
}


extern short mdl_set(char *name)
{
	short slot;
	struct model *mdl;

	/* Check if the given key is valid */
	if(!name || strlen(name) >= MDL_NAME_MAX)
		return -1;

	if((slot = mdl_get_slot()) < 0) {
		ERR_LOG(("The model-table is already full"));
		return -1;
	}

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


extern short mdl_get(char *name)
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


extern void mdl_del(short slot)
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


extern void mdl_set_mesh(short slot, int idxnum, int *idx, int vtxnum,
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


extern void mdl_set_texture(short slot, short tex)
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


extern void mdl_set_shader(short slot, short shd)
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


static int mdl_load_obj(char *pth, int *idxnum, int **idx, int *vtxnum,
		vec3_t **vtx, vec3_t **nrm, vec2_t **uv)
{
	int ret = 0, i, j, tmp;
	FILE *fd;
	char char_buf[256];
	struct dyn_stack *vtx_in = NULL, *nrm_in = NULL, *tex_in = NULL;
	struct dyn_stack *vtx_out = NULL, *nrm_out = NULL, *tex_out = NULL;
	struct dyn_stack *idx_in = NULL, *idx_conv = NULL, *idx_out = NULL;

	/* Open the file in read-mode */
	if((fd = fopen(pth, "r")) == NULL) {
		ERR_LOG(("Failed to open %s", pth));
		return -1;
	}

	/* Create buffers to read input data into */
	if(!(vtx_in = stcCreate(VEC3_SIZE))) goto err_set_ret;
	if(!(nrm_in = stcCreate(VEC3_SIZE))) goto err_set_ret;
	if(!(tex_in = stcCreate(VEC2_SIZE))) goto err_set_ret;

	/* Create buffers to write output data to */
	if(!(vtx_out = stcCreate(VEC3_SIZE))) goto err_set_ret;
	if(!(nrm_out = stcCreate(VEC3_SIZE))) goto err_set_ret;
	if(!(tex_out = stcCreate(VEC2_SIZE))) goto err_set_ret;

	/* Create buffers to store the indices */
	if(!(idx_in = stcCreate(INT3))) goto err_set_ret;
	if(!(idx_conv = stcCreate(INT3))) goto err_set_ret;
	if(!(idx_out = stcCreate(sizeof(int)))) goto err_set_ret;

	/* Read the data from the obj-file */
	while(fscanf(fd, "%s", char_buf) != EOF) {
		if(strcmp(char_buf, "vt") == 0) {
			vec2_t tex_tmp;

			/* Read texture-data */
			fscanf(fd, "%f %f", &tex_tmp[0], &tex_tmp[1]);

			/* Flip vertical-position */
			tex_tmp[1] = 1.0 - tex_tmp[1];

			stcPush(tex_in, &tex_tmp);

		}
		else if(strcmp(char_buf, "vn") == 0) {
			vec3_t nrm_tmp;

			/* Read the normal-vector */
			fscanf(fd, "%f %f %f", &nrm_tmp[0], &nrm_tmp[1],
					&nrm_tmp[2]);

			/* Push the normal into the normal-stack */	
			stcPush(nrm_in, &nrm_tmp);
		}
		else if(strcmp(char_buf, "v") == 0) {
			vec3_t vtx_tmp;

			/* Read the vector-position */
			fscanf(fd, "%f %f %f", &vtx_tmp[0], &vtx_tmp[1],
					&vtx_tmp[2]);

			/* Push the vertex into the vertex-array */
			stcPush(vtx_in, &vtx_tmp);
		}
		else if(strcmp(char_buf, "f") == 0) {
			int idx_tmp[3];

			/* Read the different indices */
			for(i = 0; i < 3; i++) {
				fscanf(fd, "%d/%d/%d", &idx_tmp[0], 
						&idx_tmp[1], &idx_tmp[2]);

				idx_tmp[0] -= 1;
				idx_tmp[1] -= 1;
				idx_tmp[2] -= 1;

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
					if(chk[1] != cur[1] || chk[2] != cur[2]) {
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
		vec3_t vtx_tmp, nrm_tmp;
		vec2_t tex_tmp;

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

err_set_ret:
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
	return (ret);
}


extern short mdl_load(char *name, char *amo, short tex, short shd)
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


extern void mdl_render(short slot, mat4_t mdl_mat)
{
	int loc[3];
	mat4_t model, view, proj;
	struct model *mdl;

	if(mdl_check_slot(slot))
		return;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		return;

	mat4_cpy(model, mdl_mat);
	cam_get_view(view);
	cam_get_proj(proj);

	glBindVertexArray(mdl->vao);
	shd_use(mdl->shd, loc);
	tex_use(mdl->tex);

	glUniformMatrix4fv(loc[0], 1, GL_FALSE, model);
	glUniformMatrix4fv(loc[1], 1, GL_FALSE, view);
	glUniformMatrix4fv(loc[2], 1, GL_FALSE, proj);

	glDrawElements(GL_TRIANGLES, mdl->idx_num, GL_UNSIGNED_INT, 0);

	tex_unuse();
	shd_unuse();
	glBindVertexArray(0);
}

