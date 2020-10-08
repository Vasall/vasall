#include "rig.h"
#include "model.h"
#include "sdl.h"
#include "mat.h"

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


static void interp(vec4_t in1, vec4_t in2, float p, vec4_t out)
{
	out[0] = in1[0] + (in2[0] - in1[0]) * p;
	out[1] = in1[1] + (in2[1] - in1[1]) * p;
	out[2] = in1[2] + (in2[2] - in1[2]) * p;
	out[3] = in1[3] + (in2[3] - in1[3]) * p;
}


static void rig_calc_rec(struct model_rig *rig, int idx);
static void rig_calc_rec(struct model_rig *rig, int idx)
{	
	struct model *mdl;
	struct mdl_anim *anim; 
	struct mdl_keyfr *keyfr0, *keyfr1;
	int i;
	vec3_t p;
	vec4_t t;
	vec4_t r;
	mat4_t mat;
	mat4_t m;
	int par;
	static int c = 0;

	mdl = models[rig->model];
	anim = &mdl->anim_buf[1];
	
	keyfr0 = &anim->keyfr_buf[(int)rig->c];
	keyfr1 = &anim->keyfr_buf[(rig->c + 1) % anim->keyfr_num];


	/* 
	 * Get the current position and rotation of the joint.
	 */
	vec3_cpy(rig->jnt_pos[idx], keyfr0->pos[idx]);
	interp(keyfr0->rot[idx], keyfr1->rot[idx], rig->prog, r);
	vec4_cpy(rig->jnt_rot[idx], r);

	/*
	 * Calculate the matrix of the joint relative to the parent.
	 */
	vec3_cpy(p, rig->jnt_pos[idx]);
	vec4_cpy(t, rig->jnt_rot[idx]);
	vec4_nrm(t, r);

	/* Add the rotation to the matrix */
	mat4_rotq(mat, r[0], r[1], r[2], r[3]);

	/* Add the position to the matrix */
	mat4_pos(mat, p[0], p[1], p[2]);

	if(c < 3) {
		printf("Relative matrix:\n");
		mat4_print(mat);
	}

	/*
	 * Add matrix to relative joint-matrix.
	 */
	mat4_mult(mdl->jnt_buf[idx].mat_rel, mat, mat);

	if(c < 3) {
		printf("Combined matrix:\n");
		mat4_print(mat);
	}

	/*
	 * Translate matrix to model-space.
	 */
	par = mdl->jnt_buf[idx].par; 
	if(par >= 0) {
		if(c < 3) {
			printf("Parent matrix(%d):\n", par);
			mat4_print(rig->jnt_mat[par]);
		}

		mat4_mult(rig->jnt_mat[par], mat, m);
	}
	else {
		mat4_cpy(m, mat);
	}

	if(c < 3) {
		printf("Absolute matrix:\n");
		mat4_print(m);
	}

	/*
	 * Copy matrix into table.
	 */
	mat4_cpy(rig->jnt_mat[idx], m);

	if(c < 3) {
		printf("--\n");
	}
	c++;

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
	static int c = 0;

	mdl = models[rig->model];
	anim = &mdl->anim_buf[0];

	/* Update the progress */
	rig->prog += 0.005;
	if(rig->prog >= 1.0) {
		rig->prog -= 1.0;
		rig->c = (rig->c + 1) % anim->keyfr_num;
	}

	/* Calculate the joint-matrices */
	rig_calc_rec(rig, mdl->jnt_root);

	/* Subtract the base matrices of the current joint-matrices */
	for(i = 0; i < mdl->jnt_num; i++) {
		if(c < 3) {
			printf("---\n");
			printf("Inverse matrix:\n");
			mat4_print(mdl->jnt_buf[i].mat);
			printf("Rig Matrix:\n");
			mat4_print(rig->jnt_mat[i]);
		}

		mat4_mul(mdl->jnt_buf[i].mat, rig->jnt_mat[i],
				rig->jnt_mat[i]);

		if(c < 3) {
			printf("Final Matrix:\n");
			mat4_print(rig->jnt_mat[i]);
		}
		c++;
	}
}
