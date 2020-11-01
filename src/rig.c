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

	/* Set the rig-attributes */
	rig->model = slot;
	rig->anim = 0;
	rig->c = 0;
	rig->prog = 0;
	rig->ts = 0;
	rig->jnt_num = mdl->jnt_num;
	rig->jnt_pos = NULL;
	rig->jnt_rot = NULL;

	/* Allocate memory for the current joint-positions */
	tmp = rig->jnt_num * VEC3_SIZE;
	if(!(rig->jnt_pos = malloc(tmp)))
		goto err_free_rig;

	/* Allocate memory for the current joint-rotations */
	tmp = rig->jnt_num * VEC4_SIZE;
	if(!(rig->jnt_rot = malloc(tmp)))
		goto err_free_arrs;

	/* Initialize joint-matrices as identitiy-matrices */
	for(i = 0; i < rig->jnt_num; i++)
		mat4_idt(rig->jnt_mat[i]);

	return rig;

err_free_arrs:
	if(rig->jnt_pos) free(rig->jnt_pos);
	if(rig->jnt_rot) free(rig->jnt_rot);

err_free_rig:
	free(rig);
	return NULL;
}


extern void rig_free(struct model_rig *rig)
{
	if(!rig)
		return;

	free(rig->jnt_rot);
	free(rig->jnt_pos);
	free(rig);
}


static void rot_interp(vec4_t in1, vec4_t in2, float p, vec4_t out)
{
	vec4_t conv;

	conv[0] = in1[0] + (in2[0] - in1[0]) * p;
	conv[1] = in1[1] + (in2[1] - in1[1]) * p;
	conv[2] = in1[2] + (in2[2] - in1[2]) * p;
	conv[3] = in1[3] + (in2[3] - in1[3]) * p;

	vec4_cpy(out, conv);
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
	mat4_t m;
	int par;

	mdl = models[rig->model];
	anim = &mdl->anim_buf[rig->anim];

	/* Get last and next keyframe */
	keyfr0 = &anim->keyfr_buf[(int)rig->c];
	keyfr1 = &anim->keyfr_buf[(rig->c + 1) % anim->keyfr_num];

	/* 
	 * Interpolate the rotation of the joint.
	 */
	rot_interp(keyfr0->rot[idx], keyfr1->rot[idx], rig->prog, r);

	/* Normalize quaternion */
	vec4_nrm(r, r);

	/* Copy rotation into rotation-list */
	vec4_cpy(rig->jnt_rot[idx], r);

	/*
	 * Interpolate the position of the joint.
	 */
	pos_interp(keyfr0->pos[idx], keyfr1->pos[idx], rig->prog, p);

	/* Copy position into position-list */
	vec3_cpy(rig->jnt_pos[idx], p);

	/*
	 * Set the current animation-matrix for the joint.
	 */

	/* Add the rotation to the matrix */
	mat4_rotq(mat, r[0], r[1], r[2], r[3]);

	/* Add the position to the matrix */
	mat4_pos(mat, p[0], p[1], p[2]);

	/*
	 * Add matrix to relative joint-matrix.
	 */
	mat4_mult(mdl->jnt_buf[idx].mat_rel, mat, mat);

	/*
	 * Translate matrix to model-space.
	 */
	par = mdl->jnt_buf[idx].par; 
	if(par >= 0)
		mat4_mult(rig->jnt_mat[par], mat, m);
	else
		mat4_cpy(m, mat);

	/*
	 * Copy matrix into table.
	 */
	mat4_cpy(rig->jnt_mat[idx], m);

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

		if(rig->c >= anim->keyfr_num) {
			rig->c = 0;
			rig->anim = (rig->anim + 1) % mdl->anim_num;
		}
	}

	/* Calculate the joint-matrices */
	rig_calc_rec(rig, mdl->jnt_root);

	/* Subtract the base matrices of the current joint-matrices */
	for(i = 0; i < mdl->jnt_num; i++) {
		mat4_mul(mdl->jnt_buf[i].mat, rig->jnt_mat[i],
				rig->jnt_mat[i]);
	}
}
