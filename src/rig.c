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

	return rig;
}


extern void rig_free(struct model_rig *rig)
{
	if(!rig)
		return;

	free(rig);
}


static void rig_reset_loc(struct model_rig *rig)
{
	int i;

	for(i = 0; i < rig->jnt_num; i++) {
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

		vec3_cpy(rig->loc_pos[i], p1);
		vec4_cpy(rig->loc_rot[i], r);
	}
}

static void rig_calc_rec(struct model_rig *rig, int idx);
static void rig_calc_rec(struct model_rig *rig, int idx)
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
	if((par = mdl->jnt_buf[idx].par) >= 0)
		mat4_mult(rig->base_mat[par], mat, mat);

	/*
	 * Write base matrix for joint.
	 */
	mat4_cpy(rig->base_mat[idx], mat);

	/*
	 * Call function recusivly for child-joints.
	 */
	for(i = 0; i < mdl->jnt_buf[idx].child_num; i++)
		rig_calc_rec(rig, mdl->jnt_buf[idx].child_buf[i]);
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
	if(p < 0) {
		keyfr[0] = 1;
		keyfr[1] = 2;
	}
	else {
		keyfr[0] = 1;
		keyfr[1] = 0;
	}

	p = ABS(p / 90.0);
	rig_calc_jnt(rig, 0, keyfr, p);

	keyfr[0] = 0;
	keyfr[1] = 1;
	rig_calc_jnt(rig, 2, keyfr, 0);

	/* 
	 * Calculate the base matrix for each joint recursivly.
	 */
	rig_calc_rec(rig, mdl->jnt_root);

	/*
	 * Subtract the base matrices of the current joint-matrices to get the 
	 * local tranform matrix.
	 */
	for(i = 0; i < mdl->jnt_num; i++) {
		mat4_mult(rig->base_mat[i], mdl->jnt_buf[i].inv_bind_mat,
				rig->tran_mat[i]);
	}
}

extern void rig_mult_mat(struct model_rig *rig, mat4_t m)
{
	int i;
	mat4_t conv_m;

	for(i = 0; i < rig->jnt_num; i++) {
		mat4_cpy(conv_m, rig->tran_mat[i]);

		mat4_mult(m, conv_m, conv_m);

		mat4_cpy(rig->tran_mat[i], conv_m);
	}
}
