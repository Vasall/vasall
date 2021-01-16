#include "rig.h"

#include "quaternion.h"
#include "model.h"
#include "sdl.h"


extern struct model_rig *rig_derive(short slot)
{
	struct model_rig *rig;
	struct model *mdl;
	int i;

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

	/* Initialize joint-matrices as identitiy-matrices */
	for(i = 0; i < rig->jnt_num; i++)
		mat4_idt(rig->jnt_mat[i]);

	return rig;
}


extern void rig_free(struct model_rig *rig)
{
	if(!rig)
		return;

	free(rig);
}

static void rig_calc_rec(struct model_rig *rig, int idx);
static void rig_calc_rec(struct model_rig *rig, int idx)
{	
	struct model *mdl;
	struct mdl_anim *anim; 
	struct mdl_keyfr *keyfr0, *keyfr1;
	int i;
	vec3_t p;
	vec4_t r;
	mat4_t mat;
	int par;

	mat4_t loc_posm;
	mat4_t loc_rotm;
	mat4_t loc_trans_mat;

	mdl = models[rig->model];
	anim = &mdl->anim_buf[rig->anim];

	/* Get last and next keyframe */
	keyfr0 = &anim->keyfr_buf[rig->keyfr];
	keyfr1 = &anim->keyfr_buf[(rig->keyfr + 1) % anim->keyfr_num];

	/* 
	 * Interpolate the position and rotation of the joint.
	 */
	vec3_interp(keyfr0->pos[idx], keyfr1->pos[idx], rig->prog, p);
	qat_interp(keyfr0->rot[idx], keyfr1->rot[idx], rig->prog, r);

	/*
	 * Set local current animation-matrix for the joint.
	 */
	mat4_idt(loc_rotm);
	mat4_fqat(loc_rotm, r[0], r[1], r[2], r[3]);
	mat4_idt(loc_posm);
	mat4_fpos(loc_posm, p[0], p[1], p[2]);
	mat4_mult(loc_posm, loc_rotm, loc_trans_mat);

	/*
	 * Add matrix to relative joint-matrix.
	 */
	mat4_mult(mdl->jnt_buf[idx].loc_bind_mat, loc_trans_mat, mat);

	/*
	 * Translate matrix to model-space.
	 */
	par = mdl->jnt_buf[idx].par; 
	if(par >= 0)
		mat4_mult(rig->jnt_mat[par], mat, mat);

	mat4_cpy(rig->jnt_mat[idx], mat);

	/*
	 * Call function recusivly for child-joints.
	 */
	for(i = 0; i < mdl->jnt_buf[idx].child_num; i++)
		rig_calc_rec(rig, mdl->jnt_buf[idx].child_buf[i]);
}

extern void rig_update(struct model_rig *rig)
{
	struct model *mdl;
	struct mdl_anim *anim;
	int i;

	mdl = models[rig->model];
	anim = &mdl->anim_buf[rig->anim];

	/* Update the progress */
	rig->prog += 0.005;
	if(rig->prog >= 1.0) {
		rig->prog -= 1.0;
		rig->keyfr = (rig->keyfr + 1);

		if(rig->keyfr >= anim->keyfr_num - 1) {
			rig->keyfr = 0;
		}
	}

	/* Calculate the joint-matrices */
	rig_calc_rec(rig, mdl->jnt_root);

	/* Subtract the base matrices of the current joint-matrices */
	for(i = 0; i < mdl->jnt_num; i++) {
		mat4_mult(rig->jnt_mat[i], mdl->jnt_buf[i].inv_bind_mat,
				rig->jnt_mat[i]);
	}
}
