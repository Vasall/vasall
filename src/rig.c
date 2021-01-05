#include "rig.h"
#include "model.h"
#include "sdl.h"


extern struct model_rig *rig_derive(short slot)
{
	struct model_rig *rig;
	struct model *mdl;
	int tmp;
	int i;

	if(mdl_check_slot(slot))
		return NULL;

	mdl = models[slot];
	if(!mdl || mdl->status != MDL_OK)
		return NULL;

	if(mdl->type < MDL_RIG)
		return NULL;

	/* Allocate memory for the rig-struct */
	if(!(rig = malloc(sizeof(struct model_rig))))
		return NULL;

	/* Initialize the rig-attributes */
	rig->model = slot;
	rig->anim = 0;
	rig->c = 0;
	rig->prog = 0;
	rig->ts = 0;
	rig->jnt_num = mdl->jnt_num;

	/* Initialize joint-matrices as identitiy-matrices */
	for(i = 0; i < rig->jnt_num; i++)
		mat4_idt(rig->jnt_mat[i]);

	return rig;

err_free_rig:
	free(rig);
	return NULL;
}


extern void rig_free(struct model_rig *rig)
{
	if(!rig)
		return;

	free(rig);
}


static void rot_interp(vec4_t in1, vec4_t in2, float p, vec4_t out)
{
	vec4_t conv = {1.0, 0.0, 0.0, 0.0};
	float dot = vec4_dot(in1, in2);
	float pi = 1.0 - p;

	if(dot < 0) {
		conv[0] = pi * in1[0] + p * -in2[0];
		conv[1] = pi * in1[1] + p * -in2[1];
		conv[2] = pi * in1[2] + p * -in2[2];
		conv[3] = pi * in1[3] + p * -in2[3];
	} else {
		conv[0] = pi * in1[0] + p * in2[0];
		conv[1] = pi * in1[1] + p * in2[1];
		conv[2] = pi * in1[2] + p * in2[2];
		conv[3] = pi * in1[3] + p * in2[3];
	}

	vec4_nrm(conv, out);
}

static void pos_interp(vec3_t in1, vec3_t in2, float p, vec3_t out)
{
	vec3_t conv;

	conv[0] = in1[0] + (in2[0] - in1[0]) * p;
	conv[1] = in1[1] + (in2[1] - in1[1]) * p;
	conv[2] = in1[2] + (in2[2] - in1[2]) * p;

	vec3_cpy(out, conv);
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
	static int c = 0;

	mat4_t loc_posm;
	mat4_t loc_rotm;
	mat4_t loc_mat;

	mdl = models[rig->model];
	anim = &mdl->anim_buf[rig->anim];

	/* Get last and next keyframe
	keyfr0 = &anim->keyfr_buf[(int)rig->c];
	keyfr1 = &anim->keyfr_buf[(rig->c + 1) % anim->keyfr_num];
	*/

	rig->prog = 1;
	keyfr0 = &anim->keyfr_buf[1];
	keyfr1 = &anim->keyfr_buf[2];

	/* 
	 * Interpolate the position and rotation of the joint.
	 */
	pos_interp(keyfr0->pos[idx], keyfr1->pos[idx], rig->prog, p);
	rot_interp(keyfr0->rot[idx], keyfr1->rot[idx], rig->prog, r);

	/*
	 * Set local current animation-matrix for the joint.
	 */
	mat4_idt(loc_rotm);
	mat4_rotq(loc_rotm, r[0], r[1], r[2], r[3]);
	mat4_idt(loc_posm);
	mat4_pos(loc_posm, p[0], p[1], p[2]);
	mat4_mult(loc_posm, loc_rotm, loc_mat);

	/*
	 * Add matrix to relative joint-matrix.
	 */
	mat4_mult(mdl->jnt_buf[idx].mat_rel, loc_mat, mat);

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
		rig->c = (rig->c + 1);

		if(rig->c >= anim->keyfr_num - 1) {
			rig->c = 0;
		}
	}

	/* Calculate the joint-matrices */
	rig_calc_rec(rig, mdl->jnt_root);

	/* Subtract the base matrices of the current joint-matrices */
	for(i = 0; i < mdl->jnt_num; i++) {
		mat4_mult(rig->jnt_mat[i], mdl->jnt_buf[i].mat_inv,
				rig->jnt_mat[i]);
	}
}
