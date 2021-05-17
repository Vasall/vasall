#include "rig.h"

#include "quaternion.h"
#include "model.h"
#include "sdl.h"


extern struct model_rig *rig_derive(short slot)
{
	struct model_rig *rig;
	struct model *mdl;

	if(mdl_check_slot(slot))
		return NULL;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		return NULL;

	if(!(mdl->attr_m & MDL_M_RIG))
		return NULL;

	/* Allocate memory for the rig-struct */
	if(!(rig = malloc(sizeof(struct model_rig))))
		return NULL;

	/* Initialize the rig-attributes */
	rig->model = slot;
	rig->anim = 0;
	rig->keyfr = 0;
	rig->prog = 0;
	rig->ts = 0;
	rig->jnt_num = mdl->jnt_num;

	/* Set hook-arrays to NULL */
	rig->hook_num = 0;
	rig->hook_pos = NULL;
	rig->hook_dir = NULL;
	rig->hook_base_mat = NULL;
	rig->hook_loc_mat = NULL;
	rig->hook_trans_mat = NULL;


	/* Add hooks to rig */
	if(mdl->hook_num > 0) {
		int tmp;
		short num;

		num = rig->hook_num = mdl->hook_num;

		/* Allocate memory for hook-positions */
		tmp = num * VEC3_SIZE; 
		if(!(rig->hook_pos = malloc(tmp)))
			goto err_free_hooks;

		/* Allocate memory for hook-forward-vectors */
		tmp = num * VEC3_SIZE;
		if(!(rig->hook_dir = malloc(tmp)))
			goto err_free_hooks;

		/* Allocate memory for hook-base-matrices */
		tmp = num * MAT4_SIZE;
		if(!(rig->hook_base_mat = malloc(tmp)))
			goto err_free_hooks;

		/* Allocate memory for hook-local-transformation-matrices */
		tmp = num * MAT4_SIZE;
		if(!(rig->hook_loc_mat = malloc(tmp)))
			goto err_free_hooks;

		/* Allocate memory for hook-transformation-matrices */
		tmp = num * MAT4_SIZE;
		if(!(rig->hook_trans_mat = malloc(tmp)))
			goto err_free_hooks;
	}

	return rig;


err_free_hooks:
	if(rig->hook_pos) free(rig->hook_pos);
	if(rig->hook_dir) free(rig->hook_dir);
	if(rig->hook_base_mat) free(rig->hook_base_mat);
	if(rig->hook_loc_mat) free(rig->hook_loc_mat);
	if(rig->hook_trans_mat) free(rig->hook_trans_mat);

err_free_rig:
	free(rig);
	return NULL;
}


extern void rig_free(struct model_rig *rig)
{
	if(!rig)
		return;

	if(rig->hook_pos) free(rig->hook_pos);
	if(rig->hook_dir) free(rig->hook_dir);
	if(rig->hook_base_mat) free(rig->hook_base_mat);
	if(rig->hook_loc_mat) free(rig->hook_loc_mat);
	if(rig->hook_trans_mat) free(rig->hook_trans_mat);

	free(rig);
}


static void rig_reset_loc(struct model_rig *rig)
{
	int i;

	for(i = 0; i < rig->jnt_num; i++) {
		rig->jnt_m[i] = 1;
		vec3_set(rig->loc_pos[i], 0, 0, 0);
		vec4_set(rig->loc_rot[i], 1, 0, 0, 0);
	}
}

static void rig_calc_jnt(struct model_rig *rig, short anim, short *keyfr,
		float prog)
{
	int i;
	struct model *mdl;
	struct mdl_anim *animp;
	struct mdl_keyfr *keyfr0;
	struct mdl_keyfr *keyfr1;
	vec3_t p0, p1, p;
	vec4_t r0, r1, r;

	mdl = models[rig->model];
	animp = &mdl->anim_buf[anim];
	
	keyfr0 = &animp->keyfr_buf[keyfr[0]];
	keyfr1 = &animp->keyfr_buf[keyfr[1]];

	/* Reset values */
	for(i = 0; i < mdl->jnt_num; i++) {
		if(keyfr0->mask[i] < 0 && keyfr1->mask[i] < 0)
			continue;

		if(keyfr0->mask[i] == -1) {
			vec3_set(p0, 0, 0, 0);
			vec4_set(r0, 1, 0, 0, 0);
		}
		else {
			vec3_cpy(p0, keyfr0->pos[i]);
			vec4_cpy(r0, keyfr0->rot[i]);
		}

		if(keyfr1->mask[i] == -1) {
			vec3_set(p1, 0, 0, 0);
			vec4_set(r1, 1, 0, 0, 0);
		}
		else {
			vec3_cpy(p1, keyfr1->pos[i]);
			vec4_cpy(r1, keyfr1->rot[i]);
		}

		vec3_interp(p0, p1, prog, p);
		qat_interp(r0, r1, prog, r);
	
		vec3_add(rig->loc_pos[i], p, rig->loc_pos[i]);
		qat_add(rig->loc_rot[i], r, rig->loc_rot[i]);
	}
}

static void rig_update_joints(struct model_rig *rig, int idx);
static void rig_update_joints(struct model_rig *rig, int idx)
{	
	struct model *mdl;
	int i;
	mat4_t mat;
	int par;

	mat4_t loc_posm;
	mat4_t loc_rotm;
	mat4_t loc_trans_mat;
	vec4_t tmp;

	mdl = models[rig->model];

	/*
	 * Set current local animation-matrix for the joint.
	 */
	mat4_idt(loc_rotm);
	vec4_cpy(tmp, rig->loc_rot[idx]);
	mat4_rfqat_s(loc_rotm, tmp[0], tmp[1], tmp[2], tmp[3]);
	mat4_idt(loc_posm);
	mat4_pfpos(loc_posm, rig->loc_pos[idx]);
	mat4_mult(loc_posm, loc_rotm, loc_trans_mat);

	/*
	 * Add matrix to relative joint-matrix.
	 */
	mat4_mult(mdl->jnt_buf[idx].loc_bind_mat, loc_trans_mat, mat);

	/*
	 * Multiply with parent matrix if joint has a parent, to convert it from
	 * local space to model space.
	 */
	if((par = mdl->jnt_buf[idx].par) != -1)
		mat4_mult(rig->base_mat[par], mat, mat);

	/*
	 * Write base matrix for joint.
	 */
	mat4_cpy(rig->base_mat[idx], mat);

	/*
	 * Calculate transformation-matrix.
	 */
	mat4_mult(mat, mdl->jnt_buf[idx].inv_bind_mat, rig->trans_mat[idx]);

	/*
	 * Call function recusivly for child-joints.
	 */
	for(i = 0; i < mdl->jnt_buf[idx].child_num; i++)
		rig_update_joints(rig, mdl->jnt_buf[idx].child_buf[i]);
}

static void rig_update_hooks(struct model_rig *rig)
{
	int i;
	struct model *mdl = models[rig->model];
	short par_jnt;
	mat4_t jnt_mat;
	mat4_t loc_mat;
	mat4_t base_mat;
	vec4_t calc;

	for(i = 0; i < rig->hook_num; i++) {
		/* Get the index of the parent-joint */
		par_jnt = mdl->hook_buf[i].par_jnt;

		/* Get the joint-matrix */
		mat4_cpy(jnt_mat, rig->base_mat[par_jnt]);

		/* Get the local hook matrix */
		mat4_cpy(loc_mat, mdl->hook_buf[i].loc_mat);

		/* Calculate the current base-matrix of the hook */
		mat4_mult(jnt_mat, loc_mat, rig->hook_base_mat[i]);

		/* Calculate the tranformation-matrix */
		mat4_mult(rig->hook_base_mat[i], mdl->hook_buf[i].inv_bind_mat,
				rig->hook_trans_mat[i]);

		/* Calculate the position of the hook */
		vec3_cpy(calc, mdl->hook_buf[i].pos);
		calc[3] = 1;
		vec4_trans(calc, rig->hook_trans_mat[i], calc);
		vec3_cpy(rig->hook_pos[i], calc);

		/* Calculate the forward-direction of the hook */
		vec3_cpy(calc, mdl->hook_buf[i].dir);
		calc[3] = 0;
		vec4_trans(calc, rig->hook_trans_mat[i], calc);
		vec3_cpy(rig->hook_dir[i], calc);
		vec3_nrm(rig->hook_dir[i], rig->hook_dir[i]);

		/* Reset the local transformation-matrix */
		mat4_idt(rig->hook_loc_mat[i]);
	}
}

extern void rig_update(struct model_rig *rig, float p)
{
	struct model *mdl;
	struct mdl_anim *anim;
	int i;
	short keyfr[2];

	mdl = models[rig->model];
	anim = &mdl->anim_buf[rig->anim];

	/* Update the progress */
	rig->prog += 0.05;
	if(rig->prog >= 1.0) {
		rig->prog -= 1.0;
		rig->keyfr = (rig->keyfr + 1);

		if(rig->keyfr >= anim->keyfr_num - 1) {
			rig->keyfr = 0;
		}
	}

	/* 
	 * Reset local position and rotation of each joint.
	 */
	rig_reset_loc(rig);

	/* 
	 * Calculate both the local position and rotation for each joint at
	 * current time.
	 */
	if(0) {
		keyfr[0] = 1;
		keyfr[1] = 2;
	}
	else {
		keyfr[0] = 0;
		keyfr[1] = 1;
	}

	rig_calc_jnt(rig, 0, keyfr, 0);

	keyfr[0] = 0;
	keyfr[1] = 1;
	rig_calc_jnt(rig, 1, keyfr, 0);

	/* 
	 * Calculate the base matrix for each joint recursivly.
	 */
	rig_update_joints(rig, mdl->jnt_root);

	/*
	 * Update hooks, if necessary.
	 */
	if(rig->hook_num > 0)
		rig_update_hooks(rig);
}


extern void rig_mult_mat(struct model_rig *rig, mat4_t m)
{
	int i;
	mat4_t conv_m;

	for(i = 0; i < rig->jnt_num; i++) {
		mat4_cpy(conv_m, rig->trans_mat[i]);

		mat4_mult(m, conv_m, conv_m);

		mat4_cpy(rig->trans_mat[i], conv_m);
	}
}


extern int rig_hk_lookat(struct model_rig *rig, short hk, vec3_t pos)
{
	int i;
	vec3_t v1;
	vec3_t v2;

	vec3_t tmp;
	float dot;
	float cross;
	mat4_t mat;

	short jnt;

	if(rig == NULL)
		return -1;

	vec3_cpy(v2, rig->hook_dir[hk]);

	vec3_sub(pos, rig->hook_pos[hk], v1);
	vec3_nrm(v1, v1);

	
	dot = vec3_dot(v1, v2);
	vec3_cross(v1, v2, tmp);
	cross = vec3_len(tmp);

	mat4_idt(mat);

	mat[0x5] = dot;
	mat[0x6] = cross;
	mat[0x9] = -cross;
	mat[0xa] = dot;

	mat4_cpy(rig->hook_loc_mat[hk], mat);

	jnt = models[rig->model]->hook_buf[0].par_jnt; 
	mat4_mult(rig->base_mat[jnt], mat, rig->base_mat[jnt]);	
	mat4_mult(rig->base_mat[jnt], models[rig->model]->jnt_buf[jnt].inv_bind_mat,
			rig->trans_mat[jnt]);

#if 0
	for(i = 0; i < models[rig->model]->jnt_buf[jnt].child_num; i++)
		rig_update_joints(rig, models[rig->model]->jnt_buf[jnt].child_buf[i]);
#endif

	return 0;
}
