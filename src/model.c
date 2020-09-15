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

	/* Initialize joint-arrays */
	mdl->jnt_buf = NULL;
	mdl->jnt_mat = NULL;
		
	/* Initialize animation-attributes */
	mdl->anim_buf = NULL;

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
	int i;
	int j;

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

	/* Free the joint-buffer */
	if(mdl->jnt_buf)
		free(mdl->jnt_buf);

	/* Free the joint-matrices */
	if(mdl->jnt_mat)
		free(mdl->jnt_mat);

	/* Free the animation-buffer and keyframes */
	if(mdl->anim_buf) {
		for(i = 0; i < mdl->anim_num; i++) {
			for(j = 0; j < mdl->anim_buf[i].keyfr_num; j++) {
				free(mdl->anim_buf[i].keyfr_buf[j].pos);
				free(mdl->anim_buf[i].keyfr_buf[j].rot);
			}

			free(mdl->anim_buf[i].keyfr_buf);
		}

		free(mdl->anim_buf);
	}

	free(mdl);
	models[slot] = NULL;
}


extern void mdl_set_mesh(short slot, int vtxnum, float *vtx, float *uv,
		float *nrm, int idxnum, unsigned int *idx)
{
	int i;
	float *ptr;
	int col_sizeb = 2;
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
	if(!(mdl->idx_buf = malloc(idxnum * sizeof(unsigned int))))
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
		memcpy(ptr + 0, vtx + (i * 3), VEC3_SIZE);
		memcpy(ptr + 3, nrm + (i * 3), VEC3_SIZE);
		memcpy(ptr + 6, uv + i * col_sizeb, col_size);
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


extern short mdl_load(char *name, char *pth, short tex_slot, short shd_slot)
{
	struct amo_model *data;
	struct model *mdl;
	short slot;

	int vtxnum;
	float *vtx;
	float *tex;
	float *nrm;
	int idxnum;
	unsigned int *idx;

	int i;
	int j;

	/* Helper-variables */
	int tmp;
	struct mdl_anim *anim;
	struct mdl_keyfr *keyfr;
	struct amo_keyfr *amo_keyfr;


	/* Allocate memory for the model-struct */
	if((slot = mdl_set(name)) < 0)
		return -1;

	/* Get pointer to model */
	mdl = models[slot]; 

	/* Load a model from a file and write it to the data-struct */
	if(!(data = amo_load(pth)))
		goto err_del_mdl;

	/* Attach both the texture and shader to the model */
	mdl_set_texture(slot, tex_slot);
	mdl_set_shader(slot, shd_slot);

	/* 
	 * Get an OpenGL-mesh from the returned data-struct. This is required as
	 * OpenGL requires that each vertex contains a position, texture-coord,
	 * and normal-vector, which has to be filled in, as the returned struct
	 * only references the attributes for each vertex via the indices.
	 * Therefore this function sorts the data and writes each attribute-data
	 * for each unique vertex to the dedicated array and returns the final
	 * index-list which can then be used by OpenGL. Note: vtx, tex, nrm all
	 * have the same number of entries.
	 */
	amo_getmesh(data, &vtxnum, (void **)&vtx, (void **)&tex, (void **)&nrm, &idxnum, &idx);

	/* Attach a mesh to the model */
	mdl_set_mesh(slot, vtxnum, vtx, tex, nrm, idxnum, idx);

	/* Free the conversion-buffers */
	free(vtx);
	free(tex);
	free(nrm);
	free(idx);

	/* Copy joints */
	if(data->jnt_c > 0) {
		/* Copy number of joints */
		mdl->jnt_num = data->jnt_c;

		printf("Load %d bones\n", mdl->jnt_num);

		/* Allocate memory for joints */
		tmp = mdl->jnt_num * sizeof(struct mdl_joint);
		if(!(mdl->jnt_buf = malloc(tmp)))
			goto err_free_data;

		/* Copy joint-data */
		for(i = 0; i < mdl->jnt_num; i++) {
			/* Copy joint-name */
			strcpy(mdl->jnt_buf[i].name, data->jnt_lst[i].name);

			/* Set parent-joint-index */
			if(data->jnt_lst[i].par)
				tmp = data->jnt_lst[i].par->index;
			else 
				tmp = -1;
			mdl->jnt_buf[i].par = tmp;			
		}

		/* Allocate memory for joint-matrices */
		tmp = mdl->jnt_num * sizeof(float) * 16;
		if(!(mdl->jnt_mat = malloc(tmp)))
			goto err_free_data;
	}

	/* Copy animations */
	if(data->ani_c > 0) {
		/* Copy number of animations */
		mdl->anim_num = data->ani_c;

		/* Allocate memory for animations */
		tmp = mdl->anim_num * sizeof(struct mdl_anim);
		if(!(mdl->anim_buf = malloc(tmp)))
			goto err_free_data;

		/* Initialize keyframe-buffers */
		for(i = 0; i < mdl->anim_num; i++) {
			/* Get shortcut-pointer */
			anim = &mdl->anim_buf[i];

			anim->keyfr_num = 0;
			anim->keyfr_buf = NULL;
		}

		/* Copy animation-keyframes */
		for(i = 0; i < mdl->anim_num; i++) {
			/* Get shortcut-pointer */
			anim = &mdl->anim_buf[i];

			/* Copy number of keyframes */
			anim->keyfr_num = data->ani_lst[i].keyfr_c;

			/* Allocate memory for keyframes */
			tmp = anim->keyfr_num * sizeof(struct mdl_keyfr);
			if(!(anim->keyfr_buf = malloc(tmp)))
				goto err_free_data;

			/* Initialize keyframe-joint-buffers */
			for(j = 0; j < anim->keyfr_num; j++) {
				/* Get shortcut-pointer */
				keyfr = &anim->keyfr_buf[j];

				keyfr->pos = NULL;
				keyfr->rot = NULL;
			}

			/* Copy keyframes */
			for(j = 0; j < anim->keyfr_num; j++) {
				/* Get shortcut-pointers */
				keyfr = &anim->keyfr_buf[j];
				amo_keyfr = &data->ani_lst[i].keyfr_lst[j];

				/* Allocate memory for location-data */
				tmp = mdl->jnt_num * sizeof(float) * 3;
				if(!(keyfr->pos = malloc(tmp)))
					goto err_free_data;

				/* Allocate memory for rotation-data */
				tmp = mdl->jnt_num * sizeof(float) * 4;
				if(!(keyfr->rot = malloc(tmp)))
					goto err_free_data;

				/* Copy timestamp */
				keyfr->ts = data->ani_lst[i].keyfr_lst[j].ts;

				/* Copy location-data */
				tmp = mdl->jnt_num * sizeof(float) * 3;
				memcpy(keyfr->pos, amo_keyfr->pos, tmp);

				/* Copy rotation-data */
				tmp = mdl->jnt_num * sizeof(float) * 4;
				memcpy(keyfr->rot, amo_keyfr->rot, tmp);
			}
		}
	}

	/* Set the status of the model */
	mdl_set_status(slot, MDL_OK);

	/* Return the returned data-struct */
	amo_destroy(data);

	/* Return the slot the model is on */
	return slot;

err_free_data:
	amo_destroy(data);

err_del_mdl:
	mdl_del(slot);
	return -1;
}


extern void mdl_render(short slot, mat4_t mat_pos, mat4_t mat_rot)
{
	int loc[4];
	mat4_t pos, rot, view, proj;
	struct model *mdl;

	if(mdl_check_slot(slot))
		return;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		return;

	mat4_cpy(pos, mat_pos);
	mat4_cpy(rot, mat_rot);
	cam_get_view(view);
	cam_get_proj(proj);

	glBindVertexArray(mdl->vao);
	shd_use(mdl->shd, loc);
	tex_use(mdl->tex);

	glUniformMatrix4fv(loc[0], 1, GL_FALSE, pos);
	glUniformMatrix4fv(loc[1], 1, GL_FALSE, rot);
	glUniformMatrix4fv(loc[2], 1, GL_FALSE, view);
	glUniformMatrix4fv(loc[3], 1, GL_FALSE, proj);

	glDrawElements(GL_TRIANGLES, mdl->idx_num, GL_UNSIGNED_INT, 0);

	tex_unuse();
	shd_unuse();
	glBindVertexArray(0);
}

