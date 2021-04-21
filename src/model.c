#include "model.h"

#include "error.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Redefine the global model-list */
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
		if(mdl->vtx_bao || mdl->vtx_bo.buffer)
			ren_destroy_buffer(mdl->vtx_bao, mdl->vtx_bo);

		if(mdl->idx_bao || mdl->idx_bo.buffer)
			ren_destroy_buffer(mdl->idx_bao, mdl->idx_bo);

		if(mdl->uni_buf || mdl->uni_bo.buffer)
			ren_destroy_buffer(mdl->uni_buf, mdl->uni_bo);

		/* Destroy vertex-array-object */
		if(mdl->vao || mdl->set)
			ren_destroy_model_data(mdl->vao, mdl->set);

		/* Free buffers */
		if(mdl->idx_buf)
			free(mdl->idx_buf);

		if(mdl->vtx_buf)
			free(mdl->vtx_buf);

		free(mdl);
	}
}


extern int mdl_check_slot(short slot)
{
	if(slot < 0 || slot >= MDL_SLOTS)
		return 1;

	return 0;
}


extern short mdl_set(char *name, short shd_slot)
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

	/* Copy the array-slot for the model */
	mdl->slot = slot;

	/* Copy the key for this model */
	strcpy(mdl->name, name);

	/* Set attribute-mask of the model */
	mdl->attr_m = MDL_M_NONE;

	/* Set the type of the model */
	mdl->type = MDL_TYPE_DEFAULT;

	/* Initialize model-attributes */
	mdl->vao = 0;
	mdl->set = VK_NULL_HANDLE;
	mdl->idx_bao = 0;
	mdl->idx_bo.buffer = VK_NULL_HANDLE;
	mdl->idx_bo.memory = VK_NULL_HANDLE;
	mdl->idx_bo.data = NULL;
	mdl->idx_bo.size = 0;
	mdl->idx_buf = NULL;
	mdl->idx_num = 0;	
	mdl->vtx_bao = 0;
	mdl->vtx_bo.buffer = VK_NULL_HANDLE;
	mdl->vtx_bo.memory = VK_NULL_HANDLE;
	mdl->vtx_bo.data = NULL;
	mdl->vtx_bo.size = 0;
	mdl->vtx_buf = NULL;
	mdl->vtx_num = 0;
	mdl->uni_buf = 0;
	mdl->uni_bo.buffer = VK_NULL_HANDLE;
	mdl->uni_bo.memory = VK_NULL_HANDLE;
	mdl->uni_bo.data = NULL;
	mdl->uni_bo.size = 0;

	/* Clear both texture and shader */
	mdl->tex = -1;
	mdl->shd = -1;

	/* Initialize joint-attributes */
	mdl->jnt_num = 0;
	mdl->jnt_buf = NULL;
	mdl->jnt_root = -1;

	/* Initialize animation-attributes */
	mdl->anim_buf = NULL;

	mdl->status = MDL_OK;

	/* Generate a new vao */
	if(ren_create_model_data(g_ast.shd.pipeline[shd_slot], &mdl->vao,
					&mdl->set) < 0)
		return -1;

	models[slot] = mdl;
	return slot;
}


extern short mdl_get(char *name)
{
	int i;

	for(i = 0; i < MDL_SLOTS; i++) {
		if(!models[i])
			continue;

		if(!strcmp(models[i]->name, name))
			return i;
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

	if(mdl->idx_bao || mdl->idx_bo.buffer)
		ren_destroy_buffer(mdl->idx_bao, mdl->idx_bo);

	if(mdl->vtx_bao || mdl->vtx_bo.buffer)
		ren_destroy_buffer(mdl->vtx_bao, mdl->vtx_bo);

	if(mdl->uni_buf || mdl->uni_bo.buffer)
		ren_destroy_buffer(mdl->uni_buf, mdl->uni_bo);

	if(mdl->vao || mdl->set)
		ren_destroy_model_data(mdl->vao, mdl->set);

	if(mdl->idx_buf)
		free(mdl->idx_buf);

	if(mdl->vtx_buf)
		free(mdl->vtx_buf);

	/* Free the joint-buffer */
	if(mdl->jnt_buf)
		free(mdl->jnt_buf);

	/* Free the animation-buffer and keyframes */
	if(mdl->anim_buf) {
		for(i = 0; i < mdl->anim_num; i++) {
			for(j = 0; j < mdl->anim_buf[i].keyfr_num; j++) {
				free(mdl->anim_buf[i].keyfr_buf[j].mask);
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


extern void mdl_set_data(short slot, int vtxnum, float *vtx, float *tex,
		float *nrm, int *jnt, float *wgt, int idxnum,
		unsigned int *idx)
{
	int i;
	char *ptr;
	int vtx_size;
	struct model *mdl;

	if(mdl_check_slot(slot))
		return;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		goto err_set_failed;


	/* Calculate the size of a single vertex in bytes */
	if(jnt && wgt) {
		/* With joints and weights */
		vtx_size = (12 * sizeof(float)) + (4 * sizeof(int));
	}
	else {
		/* Just the bare mesh(vtx-positions, uv-coord, nrm-vec) */
		vtx_size = 8 * sizeof(float);
	}

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
	ptr = mdl->vtx_buf;
	for(i = 0; i < vtxnum; i++) {
		memcpy(ptr, vtx + (i * 3), VEC3_SIZE);
		ptr += VEC3_SIZE;

		memcpy(ptr, tex + (i * 2), VEC2_SIZE);
		ptr += VEC2_SIZE;

		memcpy(ptr, nrm + (i * 3), VEC3_SIZE);
		ptr += VEC3_SIZE;

		if(mdl->attr_m & AMO_M_RIG) {
			memcpy(ptr, jnt + (i * 4), INT4_SIZE);
			ptr += INT4_SIZE;

			memcpy(ptr, wgt + (i * 4), VEC4_SIZE);
			ptr += VEC4_SIZE;
		}

	}

	if(ren_create_buffer(mdl->vao, GL_ARRAY_BUFFER, vtxnum * vtx_size,
			mdl->vtx_buf, &mdl->vtx_bao, &mdl->vtx_bo) < 0)
		goto err_set_failed;

	if(ren_create_buffer(mdl->vao, GL_ELEMENT_ARRAY_BUFFER,
			idxnum * sizeof(unsigned int), (char*)mdl->idx_buf, &mdl->idx_bao,
			&mdl->idx_bo) < 0)
		goto err_set_failed;

	if(ren_create_buffer(mdl->vao, GL_UNIFORM_BUFFER, sizeof(struct uni_buffer),
			NULL, &mdl->uni_buf, &mdl->uni_bo) < 0)
		goto err_set_failed;

	if(ren_set_model_data(mdl->vao, mdl->vtx_bao, vtx_size, jnt && wgt ? 1 : 0,
			mdl->set, mdl->uni_bo, g_ast.tex.tex[mdl->tex]) < 0)
		goto err_set_failed;

	return;

err_set_failed:
	mdl_set_status(slot, MDL_ERR_MESH);
}


extern void mdl_set_tex(short slot, short tex)
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


extern void mdl_set_shd(short slot, short shd)
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


static void mdl_order_joints(short slot)
{
	int i;
	int j;
	struct model *mdl;

	if(mdl_check_slot(slot))
		return;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		return;

	/* Clear the child-renferences */
	for(i = 0; i < mdl->jnt_num; i++) {
		mdl->jnt_buf[i].child_num = 0;

		for(j = 0; j < 10; j++)
			mdl->jnt_buf[i].child_buf[j] = -1;
	}


	for(i = 0; i < mdl->jnt_num; i++) {
		int par = mdl->jnt_buf[i].par;
		if(par < 0) {
			/* Set the root-joint */
			mdl->jnt_root = i;
			continue;
		}

		/* Add joint-index to the child-buffer of the parent-joint */
		mdl->jnt_buf[par].child_buf[mdl->jnt_buf[par].child_num] = i;
		mdl->jnt_buf[par].child_num++;
	}
}

static void mdl_calc_joint(struct model *mdl, short slot);
static void mdl_calc_joint(struct model *mdl, short slot)
{
	int i;
	struct mdl_joint *jnt;	
	mat4_t mat;

	jnt = &mdl->jnt_buf[slot];

	mat4_cpy(mat, jnt->loc_bind_mat);

	/* Adjust absolute joint-matrix using parent-joint */
	if(jnt->par >= 0)
		mat4_mult(mdl->jnt_buf[jnt->par].bind_mat, mat, mat);

	/* Attach base-matrix to joint */
	mat4_cpy(jnt->bind_mat, mat);

	/* Call function recursivly on child-joints */
	for(i = 0; i < jnt->child_num; i++)
		mdl_calc_joint(mdl, jnt->child_buf[i]);
}


extern short mdl_load(char *name, char *pth, short tex_slot, short shd_slot,
			enum mdl_type type)
{
	FILE *fd;
	short slot;

	if(!(fd = fopen(pth, "r"))) {
		ERR_LOG(("File %s doesn't exist.", pth));
		return -1;
	}

	slot = mdl_load_ffd(name, fd, tex_slot, shd_slot, type);

	fclose(fd);
	return slot;
}


extern short mdl_load_ffd(char *name, FILE *fd, short tex_slot, short shd_slot,
			enum mdl_type type)
{
	struct amo_model *data;
	struct model *mdl;
	short slot;

	int vtxnum;
	float *vtx;
	float *tex;
	float *nrm;
	int *jnt;
	float *wgt;
	int idxnum;
	unsigned int *idx;

	int i;
	int j;
	int k;

	/* Helper-variables */
	int tmp;
	struct mdl_anim *anim;
	struct mdl_keyfr *keyfr;
	struct amo_keyfr *amo_keyfr;

	/* Allocate memory for the model-struct */
	if((slot = mdl_set(name, shd_slot)) < 0)
		return -1;

	/* Get pointer to model */
	mdl = models[slot]; 

	/* Load a model from a file and write it to the data-struct */
	if(!(data = amo_load(fd)))
		goto err_del_mdl;

	/* Attach both the texture and shader to the model */
	mdl_set_tex(slot, tex_slot);
	mdl_set_shd(slot, shd_slot);

	/* Copy the attribute-mask of the model */
	mdl->attr_m = data->attr_m;

	/* Copy th type of the model */
	mdl->type = type;

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
	amo_getdata(data, &vtxnum, (void **)&vtx, (void **)&tex, (void **)&nrm,
			(void **)&jnt, (void **)&wgt, &idxnum, &idx);

	/* Attach data to the model */
	mdl_set_data(slot, vtxnum, vtx, tex, nrm, jnt, wgt, idxnum, idx);

	/* Free the conversion-buffers */
	free(vtx);
	free(tex);
	free(nrm);
	free(jnt);
	free(wgt);
	free(idx);

	/* Copy joints */
	if(data->jnt_c > 0) {
		/* Copy number of joints */
		mdl->jnt_num = data->jnt_c;

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

		/* Copy joint-matrices */
		for(i = 0; i < mdl->jnt_num; i++) {
			mat4_cpy(mdl->jnt_buf[i].loc_bind_mat,
					data->jnt_lst[i].mat);
		}

		/* Link the children to the parent-joints */
		mdl_order_joints(slot);

		/* Calculate the base matrices of the joints */
		mdl_calc_joint(mdl, mdl->jnt_root);

		/* Inverse the matrices of the joints */
		for(i = 0; i < mdl->jnt_num; i++) {
			mat4_std(mdl->jnt_buf[i].bind_mat);

			mat4_inv(mdl->jnt_buf[i].inv_bind_mat,
					mdl->jnt_buf[i].bind_mat);
		}
	}

	/* Copy animations */
	if(data->attr_m & AMO_M_ANI) {
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

			/* Copy the name of the animation */
			strcpy(anim->name, data->ani_lst[i].name);

			/* Get duration of the animation */
			anim->dur = data->ani_lst[i].dur;

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
				tmp = mdl->jnt_num * sizeof(char);
				if(!(keyfr->mask = malloc(tmp)))
					goto err_free_data;	

				/* Allocate memory for location-data */
				tmp = mdl->jnt_num * VEC3_SIZE;
				if(!(keyfr->pos = malloc(tmp)))
					goto err_free_data;

				/* Allocate memory for rotation-data */
				tmp = mdl->jnt_num * VEC4_SIZE;
				if(!(keyfr->rot = malloc(tmp)))
					goto err_free_data;

				/* Reset masks */
				for(k = 0; k < mdl->jnt_num; k++)
					keyfr->mask[k] = -1;

					
				/* Copy timestamp */
				keyfr->prog = data->ani_lst[i].keyfr_lst[j].prog;

				/* Copy position and rotation data */
				for(k = 0; k < amo_keyfr->jnt_num; k++) {
					short jnti;
					int a_off;

					/* Get the joint-index in the joint-list */
					jnti = amo_keyfr->jnt[k];

					/* mask */
					keyfr->mask[jnti] = jnti;

					/* position */
					a_off = k * 3;
					memcpy(keyfr->pos[jnti], 
							amo_keyfr->pos + a_off,
							VEC3_SIZE);

					/* rotation */
					a_off = k * 4;
					memcpy(keyfr->rot[jnti],
							amo_keyfr->rot + a_off,
							VEC4_SIZE);
				}
			}
		}
	}

	/*
	 * If handheld-hooks are defined.
	 */
	if(data->attr_m & AMO_M_HOK) {
		mdl->hok_num = data->hh_c;

		tmp = sizeof(struct amo_hook) * mdl->hok_num;
		if(!(mdl->hok_buf = malloc(tmp)))
			goto err_free_data;

		for(i = 0; i < mdl->hok_num; i++) {
			mdl->hok_buf[i].idx = data->hh_lst[i].idx;
			mdl->hok_buf[i].par_jnt = data->hh_lst[i].par_jnt;
			vec3_cpy(mdl->hok_buf[i].pos, data->hh_lst[i].pos);
		}
	}

	/*
	 * If a bounding-box is defined.
	 */
	if(data->attr_m & AMO_M_CBP) {
		/* Copy the position and size of collision-box */
		vec3_cpy(mdl->col.bb_col.pos, data->bb_col.pos);
		vec3_cpy(mdl->col.bb_col.scl, data->bb_col.scl);
	}
	
	/* 
	 * If a near-elipsoid is defined.
	 */
	if(data->attr_m & AMO_M_CNE) {
		/* Copy the position and size of collision-box */
		vec3_cpy(mdl->col.ne_col.pos, data->ne_col.pos);
		vec3_cpy(mdl->col.ne_col.scl, data->ne_col.scl);

		/* Calculate "change of basis" matrix */
		mat3_idt(mdl->col.ne_cbs);
		mdl->col.ne_cbs[0] = 1.0 / mdl->col.ne_col.scl[0];
		mdl->col.ne_cbs[4] = 1.0 / mdl->col.ne_col.scl[1];
		mdl->col.ne_cbs[8] = 1.0 / mdl->col.ne_col.scl[2];
	}

	/*
	 * If a collision-mesh is defined.
	 */
	if(data->attr_m & AMO_M_CCM) {
		/* Copy number of vertices and faces */
		mdl->col.cm_vtx_c = data->cm_vtx_c;
		mdl->col.cm_tri_c = data->cm_idx_c;

		/* Allocate memory for vertices */
		tmp = data->cm_vtx_c * VEC3_SIZE;
		if(!(mdl->col.cm_vtx = malloc(tmp)))
			goto err_free_data;

		/* Copy vertex-data */
		memcpy(mdl->col.cm_vtx, data->cm_vtx_buf, tmp);

		/* Allocate memory for indices */
		tmp = data->cm_idx_c * INT3_SIZE;
		if(!(mdl->col.cm_idx = malloc(tmp)))
			goto err_free_data;

		/* Copy index-data */
		memcpy(mdl->col.cm_idx, data->cm_idx_buf, tmp);

		/* Allocate memory for the normal-vectors */
		tmp = mdl->col.cm_tri_c * VEC3_SIZE;
		if(!(mdl->col.cm_nrm = malloc(tmp)))
			goto err_free_data;

		/* Allocate memory for the equations */
		tmp = mdl->col.cm_tri_c * VEC4_SIZE;
		if(!(mdl->col.cm_equ = malloc(tmp)))
			goto err_free_data;

		/* Calculate plane- and normal-vectors */
		for(i = 0; i < mdl->col.cm_tri_c; i++) {
			int3_t cur;
			vec3_t a;
			vec3_t b;
			vec3_t c;
			vec3_t del1;
			vec3_t del2;
			vec3_t nrm;

			memcpy(cur, mdl->col.cm_idx[i], INT3_SIZE);

			vec3_cpy(a, mdl->col.cm_vtx[cur[0]]);
			vec3_cpy(b, mdl->col.cm_vtx[cur[1]]);
			vec3_cpy(c, mdl->col.cm_vtx[cur[2]]);

			/* Calculate the plane-vectors */
			vec3_sub(b, a, del1);
			vec3_sub(c, a, del2);

			/* Calculate the normal-vector */
			vec3_cross(del1, del2, nrm);
			vec3_nrm(nrm, nrm);

			/* Copy the normal-vector */
			vec3_cpy(mdl->col.cm_nrm[i], nrm);

			/* Set the equation */
			mdl->col.cm_equ[i][0] = a[0];
			mdl->col.cm_equ[i][1] = a[1];
			mdl->col.cm_equ[i][2] = a[2];
			mdl->col.cm_equ[i][3] = -(nrm[0] * a[0] +
					nrm[1] * a[1] + nrm[2] * a[2]);
		}
	}

	/*
	 * If rig-collision-boxes are defined.
	 */
	if(data->attr_m & AMO_M_CRB) {
		/* Get the number of rig-collision-boxes */
		mdl->col.rb_c = data->rb_c;

		/* Allocate memory for the parent-joints */
		tmp = mdl->col.rb_c * sizeof(int);
		if(!(mdl->col.rb_jnt = malloc(tmp)))
			goto err_free_data;

		/* Allocate memory for the position-vectors */
		tmp = mdl->col.rb_c * VEC3_SIZE;
		if(!(mdl->col.rb_pos = malloc(tmp)))
			goto err_free_data;

		/* Allocate memory for the box-scales */
		tmp = mdl->col.rb_c * VEC3_SIZE;
		if(!(mdl->col.rb_scl = malloc(tmp)))
			goto err_free_data;

		/* Allocate memory for the box-matrices */
		tmp = mdl->col.rb_c * MAT4_SIZE;
		if(!(mdl->col.rb_mat = malloc(tmp)))
			goto err_free_data;

		/* Copy parent-joint-indices */
		tmp = mdl->col.rb_c * sizeof(int);
		memcpy(mdl->col.rb_jnt, data->rb_jnt, tmp);

		/* Copy box-position-vectors  */
		tmp = mdl->col.rb_c * VEC3_SIZE;
		memcpy(mdl->col.rb_pos, data->rb_pos, tmp);

		/* Copy box-scales */
		tmp = mdl->col.rb_c * VEC3_SIZE;
		memcpy(mdl->col.rb_scl, data->rb_scl, tmp);

		/* Copy box-matrices */
		tmp = mdl->col.rb_c * MAT4_SIZE;
		memcpy(mdl->col.rb_mat, data->rb_mat, tmp);
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


extern void mdl_render(short slot, mat4_t pos_mat, mat4_t rot_mat,
		struct model_rig *rig)
{
	mat4_t view, proj;
	struct model *mdl;
	int attr;
	struct uni_buffer uni;

	if(mdl_check_slot(slot))
		return;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		return;

	/* Get the range of vertex-attributes (0-n) */
	attr = (rig != NULL) ? (5) : (3);

	/* Get the view- and projection-matrix of the camera */
	cam_get_view(view);
	cam_get_proj(proj);

	/* Use vertices */
	ren_set_vertices(mdl->vao, mdl->vtx_bo, mdl->idx_bo);

	/* Use shader, enable vertex attributes and get uniform locations */
	shd_use(mdl->shd, attr);

	/* Set the uniform-variables */
	mat4_cpy(uni.pos_mat, pos_mat);
	mat4_cpy(uni.rot_mat, rot_mat);
	mat4_cpy(uni.view, view);
	mat4_cpy(uni.proj, proj);
	if(rig != NULL)
		memcpy(uni.tran_mat, rig->tran_mat, mdl->jnt_num * MAT4_SIZE);
	
	/* Set uniform buffer and textures */
	ren_set_render_model_data(mdl->uni_buf, uni,
			g_ast.tex.hdl[mdl->tex], g_ast.shd.pipeline[mdl->shd],
			mdl->uni_bo, mdl->set, mdl->type);

	/* Draw the vertices */
	ren_draw(mdl->idx_num, mdl->type);

	/* Unuse the texture, shader and VAO */
	/* tex_unuse();
	shd_unuse();
	glBindVertexArray(0); */
}
