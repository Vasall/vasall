#include "object_utils.h"

#include "collision.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * object-movement-log
 */

extern void obj_log_reset(short slot)
{
	g_obj.log[slot].start = 0;
	g_obj.log[slot].num = 0;
}

extern short obj_log_set(short slot, uint32_t ts, vec3_t pos, vec3_t vel,
		vec2_t mov, vec3_t dir)
{
	short i;
	short islot;
	short tmp;

	/*
	 * Check if an entry with the timestamp is already in the list. If that
	 * is the case, update the value of the entry;
	 */	
	for(i = 0; i < OBJ_LOG_LIM; i++) {
		if(g_obj.log[slot].ts[i] == ts) {
			/* Update values */
			vec3_cpy(g_obj.log[slot].pos[i], pos);
			vec3_cpy(g_obj.log[slot].vel[i], vel);

			vec2_cpy(g_obj.log[slot].mov[i], mov);
			vec3_cpy(g_obj.log[slot].dir[i], dir);
			return i;
		}
	}

	/*
	 * If the timestamp is smaller than the last entry in the list,
	 * just return.
	 */
	tmp = (g_obj.log[slot].start + g_obj.log[slot].num - 1) %
		OBJ_LOG_LIM;
	if(g_obj.log[slot].ts[tmp] > ts) {
		return -1;
	}

	/* Get the slot at the end of the list */
	islot = (g_obj.log[slot].start + g_obj.log[slot].num) %
		OBJ_LOG_LIM;

	/* Copy data to the slot */
	g_obj.log[slot].ts[islot] = ts;

	vec3_cpy(g_obj.log[slot].pos[islot], pos);
	vec3_cpy(g_obj.log[slot].vel[islot], vel);

	vec2_cpy(g_obj.log[slot].mov[islot], mov);
	vec3_cpy(g_obj.log[slot].dir[islot], dir);

	/* 
	 * Move start of list to next slot so first one can be overwritten.
	 */
	if(g_obj.log[slot].num + 1 >= OBJ_LOG_LIM) {
		g_obj.log[slot].start = (g_obj.log[slot].start + 1) %
			OBJ_LOG_LIM;
	}
	else {
		g_obj.log[slot].num += 1;
	}

	return islot;
}

/*
 * TODO: What if the timestamp is older than the oldest log-entry?!
 */
extern char obj_log_near(short slot, uint32_t ts)
{
	int i;
	char near = g_obj.log[slot].start;

	for(i = 0; i < g_obj.log[slot].num; i++) {
		short tmp = (g_obj.log[slot].start + i) % OBJ_LOG_LIM;

		if(g_obj.log[slot].ts[tmp] > ts)
			break;

		near = tmp;
	}

	return near;
}

extern void obj_log_col(uint32_t ts, char *logi)
{
	int i;

	/* Collect log-entries closest to the timestamp */
	for(i = 0; i < OBJ_LIM; i++) {
		logi[i] = -1;

		if(g_obj.mask[i] != 0) {
			logi[i] = obj_log_near(i, ts);
		}
	}
}

extern void obj_log_cpy(short slot, short i, uint32_t *ts, vec3_t pos, vec3_t vel,
		vec2_t mov, vec3_t dir)
{
	struct obj_log *log = &g_obj.log[slot];

	*ts = log->ts[i];

	vec3_cpy(pos, log->pos[i]);
	vec3_cpy(vel, log->vel[i]);

	vec2_cpy(mov, log->mov[i]);
	vec3_cpy(dir, log->dir[i]);
}


extern void obj_calc_view(short slot)
{
	int i;
	int j;
	int k;

	vec3_t pos;
	vec3_t dir;
	vec3_t off = {0, 0, 1.8};
	struct col_pck_ray pck;	

	struct model *mdl;
	vec4_t calc;
	mat4_t mat;

	/* Calculate the origin of the view-ray */
	vec3_cpy(pos, g_obj.pos[slot]);
	vec3_add(pos, off, pos);

	/* Set the objects view-offset */
	vec3_cpy(g_obj.view_origin[slot], pos);	

	/* Calculate the direction of the view-ray */
	vec3_cpy(dir, g_obj.dir[slot]);
	vec3_nrm(dir, dir);

	/* Initialize the collision-package */
	col_init_pck_ray(&pck, pos, dir);

	/* Go through all objects */
	for(i = 0; i < OBJ_LIM; i++) {
		if(g_obj.mask[i] == OBJ_M_NONE)
			continue;

		/* Don't check collision with the same object */
		if(i == slot)
			continue;

		if((g_obj.mask[i] & OBJ_M_SOLID) == 0)
			continue;

		/* Get pointer to the model */
		mdl = models[g_obj.mdl[i]];

		/* TODO: Check collision with other player-objects */
		if((mdl->attr_m & MDL_M_CCM) == 0)
			continue;

		/* Go through all triangles */
		for(j = 0; j < mdl->col.cm_tri_c; j++) {
			vec3_t vtx[3];
			int3_t idx;

			/* Get indices of triangles */
			memcpy(idx, mdl->col.cm_idx[j], INT3_SIZE);

			/* Copy corner-points */
			for(k = 0; k < 3; k++) {
				/* Copy vertex */
				vec3_cpy(vtx[k], mdl->col.cm_vtx[idx[k]]);

				/* Move relative to object-position */
				vec3_add(g_obj.pos[i], vtx[k], vtx[k]);
			}

			col_r2t_check(&pck, vtx[0], vtx[1], vtx[2]);
		}
	}


	/* Limit view-range */
	if(!pck.found || pck.col_t > 10) {
		pck.col_t = 10;
	}

	/* 
	 * Calculate the point the object is currently looking at in
	 * world-space.
	 */
	vec3_scl(dir, pck.col_t, dir);
	vec3_add(pos, dir, pos);
	vec3_cpy(g_obj.view_pos[slot], pos);

	/*
	 * Calculate the aim-point relative to the object.
	 */
	vec3_sub(pos, g_obj.pos[slot], calc);
	calc[3] = 1;
	mat4_inv(mat, g_obj.rot_mat[slot]);
	vec4_trans(calc, mat, calc);
	vec3_cpy(g_obj.view_pos_rel[slot], calc);
}


extern void obj_hnd_update(short slot)
{
	vec4_t calc;
	mat4_t mat;

	short hnd_idx;
	short par_hook;
	mat4_t hook_mat;

	/* Get the index of the handheld */
	if((hnd_idx = g_obj.hnd[slot].idx) < 0)
		return;

	/* Get the slot of the parent-hook */
	par_hook = g_hnd.par_hook[hnd_idx];	
	
	/* Get the current matrix of the parent-hook */
	mat4_cpy(hook_mat, g_obj.rig[slot]->hook_base_mat[par_hook]);

	/* Calculate the position-vector of the handheld */
	vec3_cpy(calc, g_hnd.hook_vec[hnd_idx][par_hook]);
	calc[3] = 1.0;
	vec4_trans(calc, hook_mat, calc);
	vec3_cpy(g_obj.hnd[slot].pos, calc);

	/* Calculate the position-matrix of the handheld */
	mat4_mult(hook_mat, g_hnd.hook_mat[hnd_idx][par_hook], mat);
	mat4_cpy(g_obj.hnd[slot].pos_mat, mat);

	/* Calculate the barrel-position */
	vec3_cpy(calc, g_hnd.brl_off[hnd_idx]);
	calc[3] = 1;
	vec4_trans(calc, hook_mat, calc);
	vec3_cpy(g_obj.hnd[slot].brl_pos, calc);
}
