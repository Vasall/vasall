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

	if(!(rig = malloc(sizeof(struct model_rig))))
		return NULL;

	rig->model = slot;
	rig->anim = 0;
	rig->prog = 0;
	rig->ts = 0;
	rig->jnt_num = mdl->jnt_num;
	rig->jnt_pos = NULL;
	rig->jnt_rot = NULL;
	rig->jnt_mat = NULL;

	tmp = rig->jnt_num * VEC3_SIZE;
	if(!(rig->jnt_pos = malloc(tmp)))
		goto err_free_rig;

	tmp = rig->jnt_num * VEC4_SIZE;
	if(!(rig->jnt_rot = malloc(tmp)))
		goto err_free_arrs;

	tmp = rig->jnt_num * MAT4_SIZE;
	if(!(rig->jnt_mat = malloc(tmp)))
		goto err_free_rig;

	/* Initialize joint-matrices as identitiy-matrices */
	for(i = 0; i < rig->jnt_num; i++)
		mat4_idt(rig->jnt_mat[i]);

	return rig;

err_free_arrs:
	if(rig->jnt_pos) free(rig->jnt_pos);
	if(rig->jnt_rot) free(rig->jnt_rot);
	if(rig->jnt_mat) free(rig->jnt_mat);

err_free_rig:
	free(rig);
	return NULL;
}


extern void rig_free(struct model_rig *rig)
{
	if(!rig)
		return;

	free(rig->jnt_mat);
	free(rig);
}


extern void rig_update(struct model_rig *rig)
{
	struct model *mdl;
	struct mdl_keyfr *keyfr;
	int i;
	mat4_t mat;
	static char c = 0;

	mdl = models[rig->model];
	keyfr = &mdl->anim_buf[0].keyfr_buf[1];

	/* Get the current position and rotation for each joint */
	for(i = 0; i < rig->jnt_num; i++) {
		vec3_cpy(rig->jnt_pos[i], keyfr->pos[i]);
		vec3_cpy(rig->jnt_rot[i], keyfr->rot[i]);
	}

	/* Fill in the matrices */
	for(i = 0; i < rig->jnt_num; i++) {
		vec3_t p;
		vec4_t r;

		vec3_cpy(p, rig->jnt_pos[i]);
		vec4_cpy(r, rig->jnt_rot[i]);

#if 0
		mat[0] = 1.0 - 2.0 * (r[1] * r[1] + r[2] * r[2]);
		mat[1] = 2.0 * (r[0] * r[1] - r[2] * r[3]);
		mat[2] = 2.0 * (r[0] * r[2] + r[1] * r[3]);
		mat[3] = 0;
		
		mat[4] = 2.0 * (r[0] * r[1] + r[2] * r[3]);
		mat[5] = 1.0 - 2.0 * (r[0] * r[0] + r[2] * r[2]);
		mat[6] = 2.0 * (r[1] * r[2] - r[0] * r[3]);
		mat[7] = 0;

		mat[8] = 2.0 * (r[0] * r[2] - r[1] * r[3]);
		mat[9] = 2.0 * (r[1] * r[2] + r[0] * r[3]);
		mat[10] = 1.0 - 2.0 * (r[0] * r[0] + r[1] * r[1]);
		mat[11] = 0;
		
		mat[12] = p[0];
		mat[13] = p[1];
		mat[14] = p[2];
		mat[15] = 1;
#endif

		if(c == 0) {
			c = 1;
			mat4_print(mat);
		}

		mat4_cpy(rig->jnt_mat[i], mat);
	}
}
