#include "object.h"

#include "world.h"
#include "network.h"
#include "collision.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Redefine global object-wrapper */
struct object_table objects;


extern int obj_init(void)
{
	int i;

	for(i = 0; i < OBJ_LIM; i++) {
		objects.order[i] = i;

		objects.mask[i] = OBJ_M_NONE;
	}

	objects.num = 0;

	return 0;
}


/* TODO */
extern void obj_close(void)
{
	return;
}


static short obj_get_slot(void)
{
	short i;

	for(i = 0; i < OBJ_LIM; i++) {
		if(objects.mask[i] == OBJ_M_NONE)
			return i;
	}

	return -1;
}


static int obj_check_slot(short slot)
{
	if(slot < 0 || slot > OBJ_LIM)
		return 1;

	return 0;
}

static void obj_sort(void)
{
	int i;
	char found = 0;

	short a;
	short b;

	for(i = 0; i < OBJ_LIM; i++) {
		objects.order[i] = i;
	}

	/*
	 * TODO:
	 * I did it again. uwu
	 * Bubblesort owo, how could this happen...
	 */

	/* Sort object-order to ascending IDs. */
	while(1) {
		found = 0;

		for(i = 0; i < OBJ_LIM - 1; i++) {
			a = objects.order[i];
			b = objects.order[i + 1];

			if(objects.mask[a] != 0 && objects.mask[b] != 0) {
				if(objects.id[a] > objects.id[b]) {
					found = 1;

					objects.order[i] = b;
					objects.order[i + 1] = a;
				}
			}
			else if(objects.mask[a] == 0 && objects.mask[b] != 0) {
				found = 1;

				objects.order[i] = b;
				objects.order[i + 1] = a;
			}
		}

		if(!found)
			break;
	}
}

extern short obj_set(uint32_t id, uint32_t mask, vec3_t pos, short model,
		char *data, int len, uint32_t ts)
{
	short slot;


	/* TODO: Get altitude at current position */
	pos[2] = 0.0;


	/* Get a slot to place object on */
	if((slot = obj_get_slot()) < 0)
		return -1;

	/* Copy the valued and initialize the attributes */
	objects.mask[slot] = mask;
	objects.id[slot] = id;

	/* Set the timestamp of the object */
	objects.ts[slot] = ts;

	printf("Position %d: ", id);
	vec3_print(pos);
	printf("\n");

	/* Setup position and velocity */
	vec3_cpy(objects.pos[slot], pos);
	vec3_clr(objects.vel[slot]);

	/* Set movement and direction */
	vec2_clr(objects.mov[slot]);
	vec3_set(objects.dir[slot], 0, 1, 0);

	/* Initialize interpolation variables */
	objects.prev_ts[slot] = objects.ts[slot];
	vec3_cpy(objects.prev_pos[slot], objects.pos[slot]);
	vec3_cpy(objects.prev_dir[slot], objects.dir[slot]);

	/* Initialize the movement-log */
	obj_log_reset(slot);
	obj_log_set(slot, objects.ts[slot], objects.pos[slot],
			objects.vel[slot], objects.mov[slot],
			objects.dir[slot]);

	/* Initialize object model and rig if requested */
	objects.mdl[slot] = -1;
	if(mask & OBJ_M_MODEL) {
		objects.mdl[slot] = model;

		/* Attach a rig to the object */
		objects.rig[slot] = NULL;
		if(mask & OBJ_M_RIG) {
			if(models[model]->attr_m & MDL_M_RIG) {
				if(!(objects.rig[slot] = rig_derive(model)))
					goto err_reset_slot;
			}
		}
	}

	/* Initialize the position and rotation matrices */
	obj_update_matrix(slot);

	/* Initialize data-buffer if requested */
	objects.len[slot] = 0;
	if(mask & OBJ_M_DATA) {
		if(data && len) {
			len = (len > OBJ_DATA_MAX) ? (OBJ_DATA_MAX) : (len);
			objects.len[slot] = len;
			memcpy(objects.data[slot], data, len);
		}
	}

	/* Increment number of objects in the object-table */
	objects.num++;

	/* Sort the order of objects in the object-list */
	obj_sort();

	/* Sort the objects */
	return slot;

err_reset_slot:
	objects.mask[slot] = OBJ_M_NONE;
	return -1;
}


extern void obj_del(short slot)
{
	if(obj_check_slot(slot))
		return;

	objects.mask[slot] = OBJ_M_NONE;
	objects.num--;
}


extern int obj_attach_rig(short slot, short mdlslot)
{
	struct model_rig *rig;

	if(obj_check_slot(slot))
		return -1;

	if(mdl_check_slot(mdlslot))
		return -1;

	if(!(rig = rig_derive(mdlslot)))
		return -1;

	objects.rig[slot] = rig;	
	return 0;
}


extern int obj_mod(short slot, short attr, void *data, int len)
{
	if(obj_check_slot(slot) || objects.mask[slot] == OBJ_M_NONE)
		return -1;

	switch(attr) {
		case OBJ_A_MASK:
			objects.mask[slot] = *(uint32_t *)data;
			break;

		case OBJ_A_POS:
			vec3_cpy(objects.pos[slot], data);
			break;

		case OBJ_A_VEL:
			vec3_cpy(objects.vel[slot], data);
			break;

		case OBJ_A_DIR:
			vec3_cpy(objects.dir[slot], data);
			break;

		case OBJ_A_BUF:
			objects.len[slot] = len;
			memcpy(objects.data[slot], data, len);
			break;

		default:
			return -1;
	}

	return 0;
}


extern short obj_sel_id(uint32_t id)
{
	short i;

	for(i = 0; i < OBJ_LIM; i++) {
		if(objects.mask[i] == OBJ_M_NONE)
			continue;

		if(objects.id[i] == id)
			return i;
	}

	return -1;
}


extern void obj_update_matrix(short slot)
{
	float rot;

	if(obj_check_slot(slot))
		return;

	/* Reset the model-matrices to identity-matrices */
	mat4_idt(objects.mat_pos[slot]);
	mat4_idt(objects.mat_rot[slot]);

	/* Set the position of the model */
	objects.mat_pos[slot][0xc] = objects.pos[slot][0];
	objects.mat_pos[slot][0xd] = objects.pos[slot][1];
	objects.mat_pos[slot][0xe] = objects.pos[slot][2];

	/* Set the rotation of the model */
	rot = atan2(objects.dir[slot][1], objects.dir[slot][0]);
	objects.mat_rot[slot][0x0] =  cos(rot);
	objects.mat_rot[slot][0x1] =  sin(rot);
	objects.mat_rot[slot][0x4] = -sin(rot);
	objects.mat_rot[slot][0x5] =  cos(rot);
}


extern int obj_list(void *ptr, short *num, short max)
{
	short obj_num = 0;
	short i;

	char *buf_ptr = (char *)ptr + 2;

	for(i = 0; i < OBJ_LIM && obj_num < max; i++) {
		if(objects.mask[i] == OBJ_M_NONE)
			continue;

		/* Write the id */
		memcpy(buf_ptr, &objects.id[i], 4);
		buf_ptr += 4;

		obj_num++;
	}	

	if(obj_num == 0)
		return 0;

	if(num != NULL)
		*num = obj_num;

	memcpy(ptr, &obj_num, 2);
	return (obj_num * sizeof(uint32_t)) + 2;
}


extern int obj_collect(uint16_t flg, void *in, short in_num, void **out,
		short *out_num)
{
	int i;
	uint32_t *id_ptr = (uint32_t *)in;
	int written = 0;
	short slot;

	char *out_buf;
	char *ptr;

	short num = 0;

	/* Allocate memory for the return-buffer */
	if(!(out_buf = malloc(in_num * 44 + 2)))
		return 0;

	/* Set the starting-pointer to write the object-data */
	ptr = out_buf + 2;

	for(i = 0; i < in_num; i++) {
		if((slot = obj_sel_id(*id_ptr)) >= 0) {
			/* Copy object-id */
			if(flg & OBJ_A_ID) {
				memcpy(ptr, id_ptr, 4);
				ptr += 4;
				written += 4;
			}

			/* Copy object-mask */
			if(flg & OBJ_A_MASK) {
				memcpy(ptr, &objects.mask[slot], 4);
				ptr += 4;
				written += 4;
			}

			/* Copy the position */
			if(flg & OBJ_A_POS) {
				vec3_cpy((float *)ptr, objects.pos[slot]);
				ptr += VEC3_SIZE;
				written += VEC3_SIZE;
			}

			/* Copy the velocity of the object */
			if(flg & OBJ_A_VEL) {
				vec3_cpy((float *)ptr, objects.vel[slot]);
				ptr += VEC3_SIZE;
				written += VEC3_SIZE;
			}

			/* Copy the mov-force of the object */
			if(flg & OBJ_A_MOV) {
				vec2_cpy((float *)ptr, objects.mov[slot]);
				ptr += VEC2_SIZE;
				written += VEC2_SIZE;
			}

			if(flg & OBJ_A_BUF) {
				int tmp = objects.len[slot];

				memcpy(ptr, &tmp, 4);
				ptr += 4;
				written += 4;

				memcpy(ptr, objects.data[slot], tmp);
				ptr += tmp;
				written += tmp;
			}

			/* Increment the number of objects */
			num++;
		}

		/* Go to the next id */
		id_ptr++;
	}

	/* Copy the object-number and the timestamp into the buffer */
	memcpy(out_buf, &num, 2);

	/* Set return-variables */
	*out = out_buf;
	if(out_num) *out_num = num;

	/* Return the number of written bytes */
	return written + 2;
}


extern int obj_submit(void *in, uint32_t ts)
{
	uint32_t id;
	uint32_t mask;
	vec3_t pos;
	short slot;
	short mdl;
	char *ptr = in;

	/* Extract data from the packet */
	memcpy(&id,    ptr,       4);
	memcpy(&mask,  ptr +  4,  4);
	vec3_cpy(pos, (float *)(ptr +  8));

	mdl = mdl_get("plr");

	if((slot = obj_set(id, mask, pos, mdl, NULL, 0, ts)) < 0)
		return -1;

	return 0;
}


extern int obj_update(void *in)
{
	char *ptr = in;
	uint8_t flg;

	memcpy(&flg, ptr, 1);
	ptr += 1;

	return 0;
}


/* Collect all triangles the object collides with */
static void checkCollision(struct col_pck_sphere *pck)
{
	int i;
	int j;
	int k;

	struct model *mdl;

	/* Go through all objects */
	for(i = 0; i < OBJ_LIM; i++) {
		if(objects.mask[i] == OBJ_M_NONE)
			continue;

		/* Don't check collision with the same object */
		if(i == pck->objSlot)
			continue;

		if((objects.mask[i] & OBJ_M_SOLID) == 0)
			continue;

		/* Get pointer to the model */
		mdl = models[objects.mdl[i]];

		/* TODO: Check collision with other player-objects */
		if((mdl->attr_m & MDL_M_CCM) == 0)
			continue;

		/* Go through all triangles */
		for(j = 0; j < mdl->col.cm_tri_c; j++) {
			vec3_t vtx[3];
			int3_t idx;

			/* Get indices of triangles */
			memcpy(idx, mdl->col.cm_idx[j], INT3_SIZE);

			/* Load triangle vertices and convert to eSpace */
			for(k = 0; k < 3; k++) {
				vec3_t tmpv;

				/* Copy vertex */
				vec3_cpy(tmpv, mdl->col.cm_vtx[idx[k]]);
				vec3_add(objects.pos[i], tmpv, vtx[k]);

				/* Convert to eSpace */
				vec3_div(vtx[k], pck->eRadius, vtx[k]);
			}

			col_s2t_check(pck, vtx[0], vtx[1], vtx[2]);
		}
	}
}

static void collideWithWorld(struct col_pck_sphere *pck, vec3_t pos, vec3_t del, int recDepth, vec3_t *opos)
{
	float unitsPerMeter = 100.0;
	float unitScale = unitsPerMeter / 100.0;
	float veryCloseDistance = 0.005f * unitScale;

	vec3_t destPoint;
	vec3_t newBasePoint;

	vec3_t slidePlaneOrigin;
	vec3_t slidePlaneNormal;
	struct col_pln slidingPlane;
	vec3_t newDestinationPoint;
	float tmpDist;
	vec3_t newVelocityVector;

	/* Limit recusrion-depth */
	if(recDepth > 5) {
		vec3_cpy(*opos, pos);
		return;
	}

	/* Fill collision-packet with the necessary data */
	vec3_cpy(pck->velocity, del);
	vec3_nrm(del, pck->normalizedVelocity);
	vec3_cpy(pck->basePoint, pos);
	pck->foundCollision = 0;

	/* Check if a collision occurred and calculate the collision-point */
	checkCollision(pck);

	/* If no collision has been found */
	if(pck->foundCollision == 0) {
		vec3_add(pos, del, *opos);
		return;
	}

	vec3_add(pos, del, destPoint);
	vec3_cpy(newBasePoint, pos);

	if(pck->nearestDistance >= veryCloseDistance) {
		vec3_t v;
		float tf;

		tf = MIN(vec3_len(del), pck->nearestDistance - veryCloseDistance);
		vec3_setlen(del, tf, v);

		vec3_add(v, pck->basePoint, newBasePoint);

		/*
		 * Adjust polygon intersection point (so sliding plane will be
		 * unaffected by the fact that we move slightly less than
		 * collision tells us)
		 */
		vec3_setlen(v, veryCloseDistance, v);
		vec3_sub(pck->colPnt, v, pck->colPnt);
	}

	/* Determine the sliding plane */
	vec3_cpy(slidePlaneOrigin, pck->colPnt);
	vec3_sub(newBasePoint, pck->colPnt, slidePlaneNormal);
	vec3_nrm(slidePlaneNormal, slidePlaneNormal);
	col_pln_fnrm(&slidingPlane, slidePlaneOrigin, slidePlaneNormal);


	/* Calculate the new destination point */
	tmpDist = col_pln_dist(&slidingPlane, destPoint);	
	vec3_scl(slidePlaneNormal, tmpDist, slidePlaneNormal);
	vec3_sub(destPoint, slidePlaneNormal, newDestinationPoint);

	/*
	 * Generate the slide vector, which will become our new velocity vector
	 * for the next iteration.
	 */
	vec3_sub(newDestinationPoint, pck->colPnt, newVelocityVector);

	/*
	 * Recursion:
	 */
	if(vec3_len(newVelocityVector) < veryCloseDistance) {
		vec3_cpy(*opos, newBasePoint);
		return;
	}

	recDepth++;
	collideWithWorld(pck, newBasePoint, newVelocityVector, recDepth, opos);
}

static int collideAndSlide(short slot, vec3_t pos, vec3_t del, vec3_t opos)
{		
	struct col_pck_sphere pck;

	vec3_t relpos;
	vec3_t epos;
	vec3_t edel;
	vec3_t retPos;

	unsigned int retMask = 0;

	/*
	 * Get the centerpoint of the ne-collision-sphere by adding the offset
	 * to the current position of the object.
	 */
	vec3_add(pos, models[objects.mdl[slot]]->col.ne_col.pos, relpos);

	/*
	 * Set object slot, so the object won't check itself for collision.
	 */
	pck.objSlot = slot;

	/*
	 * Copy the scaling-factors of the sphere, which will be used for
	 * conversion from R3-Space to eSpace.
	 */
	vec3_cpy(pck.eRadius, models[objects.mdl[slot]]->col.ne_col.scl);

	/*
	 * Copy both the position and the movement in R3-Space.
	 */
	vec3_cpy(pck.R3Pos, relpos);
	vec3_cpy(pck.R3Vel, del);

	/*
	 * Convert position and movement to eSpace.
	 */
	vec3_div(relpos, pck.eRadius, epos);
	vec3_div(del, pck.eRadius, edel);

	/*
	 * Check for collision with the triangle.
	 */
	collideWithWorld(&pck, epos, edel, 0, &retPos);

	if(pck.foundCollision) {
		retMask = 1;
	}

	/*
	 * Convert position back to R3-Space and subtract offset.
	 */
	vec3_mult(retPos, pck.eRadius, opos);
	vec3_sub(opos, models[objects.mdl[slot]]->col.ne_col.pos, opos);

	/*
	 * Return if a collision occurred.
	 */
	return retMask;
}


extern void obj_calc_aim(short slot)
{
	int i;
	int j;
	int k;

	vec3_t pos;
	vec3_t dir;
	vec3_t off = {0, 0, 1.8};
	struct col_pck_ray pck;	

	struct model *mdl;

	/* Calculate the origin of the aiming-ray */
	vec3_cpy(pos, objects.pos[slot]);
	vec3_add(pos, off, pos);

	/* Calculate the direction of the aiming-ray */
	vec3_cpy(dir, objects.dir[slot]);
	vec3_nrm(dir, dir);

	/* Initialize the collision-package */
	col_init_pck_ray(&pck, pos, dir);

	/* Go through all objects */
	for(i = 0; i < OBJ_LIM; i++) {
		if(objects.mask[i] == OBJ_M_NONE)
			continue;

		/* Don't check collision with the same object */
		if(i == slot)
			continue;

		if((objects.mask[i] & OBJ_M_SOLID) == 0)
			continue;

		/* Get pointer to the model */
		mdl = models[objects.mdl[i]];

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
			for(k = 0; k < 3; k++)
				vec3_cpy(vtx[k], mdl->col.cm_vtx[idx[k]]);

			col_r2t_check(&pck, vtx[0], vtx[1], vtx[2]);
		}
	}


	/* Limit aiming-range */
	if(pck.found) {
		if(pck.col_t > 10) {
			pck.col_t = 10;
		}
	}
	else {
		pck.col_t = 10;
	}


	/* Calculate the point the object is currently aiming at */
	vec3_scl(dir, pck.col_t, dir);
	vec3_add(pos, dir, pos);
	vec3_cpy(objects.aim_pos[slot], pos);
}


extern void obj_print(short slot)
{
	if(obj_check_slot(slot))
		return;

	printf("Display object %d:\n", slot);
	printf("Pos: "); vec3_print(objects.pos[slot]); printf("\n");
	printf("Vel: "); vec3_print(objects.vel[slot]); printf("\n");
	printf("Dir: "); vec3_print(objects.dir[slot]); printf("\n");
	printf("Mov: "); vec2_print(objects.mov[slot]); printf("\n");
}

/*
 * ============================================
 *                OBJECT_SYSTEMS
 * ============================================
 */

static void obj_interp(short slot, float interp)
{
	vec3_t del;

	/* Update render-position of the object */
	vec3_sub(objects.pos[slot], objects.prev_pos[slot], del);
	vec3_scl(del, interp, del);
	vec3_add(objects.prev_pos[slot], del, objects.ren_pos[slot]);

	/* Store last position */
	vec3_cpy(objects.prev_pos[slot], objects.pos[slot]);

	/* Update the direction of the object */
	vec3_sub(objects.dir[slot], objects.prev_dir[slot], del);
	vec3_scl(del, interp, del);
	vec3_add(objects.prev_dir[slot], del, objects.ren_dir[slot]);

	/* Store last direction */
	vec3_cpy(objects.prev_dir[slot], objects.dir[slot]);
}

static void obj_proc_rig_fpv(short slot)
{
	vec3_t off_v = {0, 0, -1.6};
	vec3_t rev_v = {0, 0, 1.8};

	mat4_t off_m;
	mat4_t rev_m;
	mat4_t trans_m;

	/* Calculate rig without aiming */
	rig_update(objects.rig[slot], 0);

	/* 
	 * Calculate transformation-matrix for
	 * first-person-view.
	 */
	mat4_idt(off_m);
	mat4_idt(rev_m);
	mat4_pfpos(off_m, off_v);
	mat4_pfpos(rev_m, rev_v);

	mat4_cpy(trans_m, camera.forw_m);
	mat4_mult(trans_m, off_m, trans_m);
	mat4_mult(rev_m, trans_m, trans_m);

	/*
	 * Multiply transformation-matrix with
	 * joint-matrices.
	 */
	rig_mult_mat(objects.rig[slot], trans_m);
}

static void obj_proc_rig_tpv(short slot)
{
	float agl;
	vec3_t up = {0, 0, 1};
	vec3_t dir;
	float rot;
	mat4_t trans_m;

	agl = vec3_angle(up, objects.dir[slot]);
	agl = 90.0 - RAD_TO_DEG(agl);

	/* Calculate rig with aiming */
	rig_update(objects.rig[slot], agl);

	vec2_set(dir, objects.dir[slot][0], objects.dir[slot][1]);
	vec2_nrm(dir, dir);

	/* Set the rotation of the model */
	rot = RAD_TO_DEG(atan2(-dir[0], dir[1]));

	mat4_idt(trans_m);
	mat4_rfagl_s(trans_m, 0, 0, rot);

	rig_mult_mat(objects.rig[slot], trans_m);
}

static void obj_update_aim(short slot)
{
	mat4_t mat;
	short hook;
	short jnt;
	vec4_t pos;
	vec4_t dir;


	/* 
	 * Render the weapon.
	 */		
	hook = handhelds.hook[0];
	jnt = models[objects.mdl[slot]]->hok_buf[hook].par_jnt;

	mat4_cpy(mat, objects.rig[slot]->tran_mat[jnt]);

	vec4_clr(pos);
	pos[3] = 1.0;
	vec3_cpy(pos, handhelds.aim_pos[0]);

	vec4_clr(dir);
	vec3_cpy(dir, handhelds.aim_dir[0]);

	vec4_trans(pos, mat, pos);
	vec4_trans(dir, mat, dir);
	vec3_nrm(dir, dir);

	vec3_cpy(objects.aim_off[slot], pos);
	vec3_cpy(objects.aim_dir[slot], dir);
}

extern void obj_sys_update(uint32_t now)
{
	int i;
	int o;

	uint32_t lim_ts;
	uint32_t run_ts;
	uint32_t inp_ts;

	char logi[OBJ_LIM];

	float f;
	vec3_t acl;
	vec3_t del;

	vec3_t frw;
	vec3_t rgt;
	vec3_t up = {0, 0, 1};

	float t_speed = 4.0;
	vec3_t grav = {0, 0, -9.81};

	struct input_entry inp;

	int c = 0;

	now = floor(now / TICK_TIME) * TICK_TIME;

	/* Check if new inputs occurred */
	if(inp_check_new()) {
		/* Set iterator to latest input */
		inp_begin();

		inp_ts = inp_cur_ts();
		run_ts = inp_ts;

		obj_log_col(inp_ts, logi);

		for(i = 0; i < OBJ_LIM; i++) {
			if((objects.mask[i] & OBJ_M_MOVE) == 0)
				continue;

			if(objects.ts[i] > inp_ts) {
				obj_log_cpy(i, logi[i],
						&objects.ts[i],
						objects.pos[i],
						objects.vel[i],
						objects.mov[i],
						objects.dir[i]);
			}

			/* Update run-ts to the oldest timestamp */
			if(run_ts > objects.ts[i]) {
				run_ts = objects.ts[i];
			}
		}

		/* Set the run-limit */	
		lim_ts = inp_ts;
	}
	else {
		for(i = 0; i < OBJ_LIM; i++) {
			if((objects.mask[i] & OBJ_M_MOVE) == 0)
				continue;

			/* Update run-ts to the oldest timestamp */
			if(run_ts > objects.ts[i]) {
				run_ts = objects.ts[i];
			}
		}

		/* Set the run-limit */
		lim_ts = now;
	}


	while(1) {
		while(run_ts < lim_ts) {
			c++;

			/*
			 * Update the velocity and position of every object in
			 * ascending order of the object-ID so collisions will
			 * be processed equally on all clients.
			 */
			for(i = 0; i < objects.num; i++) {
				o = objects.order[i];

				if((objects.mask[o] & OBJ_M_MOVE) == 0)
					continue;

				/* 
				 * Skip if the object doesn't have to be updated
				 * yet.
				 */
				if(run_ts < objects.ts[o])
					continue;

				/* 
				 * Process friction.
				 */
				f = 1.0 - TICK_TIME_S * t_speed;
				vec3_scl(objects.vel[o], f, objects.vel[o]);

				/*
				 * Process movement-acceleration.
				 */

				/* Calculate the direction of acceleration */
				vec3_set(frw, objects.dir[o][0], objects.dir[o][1], 0);
				vec3_nrm(frw, frw);

				vec3_cross(frw, up, rgt);
				vec3_nrm(rgt, rgt);

				vec3_scl(frw, objects.mov[o][1], frw);
				vec3_scl(rgt, objects.mov[o][0], rgt);

				vec3_add(frw, rgt, acl);
				vec3_nrm(acl, acl);

				/* Scale acceleration */
				vec3_scl(acl, 6, acl);

				f = TICK_TIME_S * t_speed;
				vec3_scl(acl, f, acl);

				/* Update velocity of the object */
				vec3_add(objects.vel[o], acl, objects.vel[o]);

				/*
				 * Process gravity.
				 */	
				if(objects.mask[o] & OBJ_M_GRAV) {
					f = TICK_TIME_S * t_speed;
					vec3_scl(grav, f, acl);

					/* Update velocity of the object */
					vec3_add(objects.vel[o], acl, objects.vel[o]);
				}

				/* Scale velocity by tick-time */
				vec3_scl(objects.vel[o], TICK_TIME_S, del);
				del[2] = 0.0;

				/* Check collision */
				if(objects.mask[o] & OBJ_M_SOLID) {
					/* Collide and update position */
					collideAndSlide(o, objects.pos[o],
							del, objects.pos[o]);
				}
				else {
					/* Update position */
					vec3_add(objects.pos[o], del,
							objects.pos[o]);
				}


				if(objects.mask[o] & OBJ_M_GRAV) {
					/* Scale velocity by tick-time */
					vec3_scl(objects.vel[o], TICK_TIME_S, del);
					del[0] = 0.0;
					del[1] = 0.0;

					/* Check collision */
					if(objects.mask[o] & OBJ_M_SOLID) {
						/* Collide and update position */
						if(collideAndSlide(o, objects.pos[o],
									del, objects.pos[o])) {
							objects.vel[o][2] = 0;
						}
					}
					else {
						/* Update position */
						vec3_add(objects.pos[o], del,
								objects.pos[o]);
					}
				}

				/* Limit movement-space */
				if(ABS(objects.pos[o][0]) > 32.0) {
					objects.pos[o][0] = 32.0 * SIGN(objects.pos[o][0]);
					objects.vel[o][0] = 0;
				}

				if(ABS(objects.pos[o][1]) > 32.0) {
					objects.pos[o][1] = 32.0 * SIGN(objects.pos[o][1]);
					objects.vel[o][1] = 0;
				}

				objects.ts[o] += TICK_TIME;

				/*   */
				if((objects.ts[o] % OBJ_LOG_TIME) == 0) {
					obj_log_set(o,
							objects.ts[o],
							objects.pos[o],
							objects.vel[o],
							objects.mov[o],
							objects.dir[o]);


				}
			}

			/*
			 * Update run-timer.
			 */
			run_ts += TICK_TIME;
		}

		if(inp_get(&inp)) {
			short obj_slot = obj_sel_id(inp.obj_id);

			if(inp.ts >= objects.ts[obj_slot]) {
				if(inp.mask & INP_M_MOV)
					vec2_cpy(objects.mov[obj_slot], inp.mov);

				if(inp.mask & INP_M_DIR)
					vec3_cpy(objects.dir[obj_slot], inp.dir);
			}

			/* Jump to next input */
			if(inp_next()) {
				if((lim_ts = inp_cur_ts()) == 0)
					lim_ts = now;
			}
			else {
				lim_ts = now;
			}
		}
		else {
			if(run_ts >= now)
				break;

			lim_ts = now;
		}
	}
}


extern void obj_sys_prerender(float interp)
{
	int i;
	vec3_t del;
	mat4_t trans_m;

	for(i = 0; i < OBJ_LIM; i++) {
		if(objects.mask[i] & OBJ_M_MODEL) {
			obj_interp(i, interp);
		}

		if(objects.mask[i] & OBJ_M_RIG) {
			if(i == core.obj && camera.mode == CAM_MODE_FPV) {
				obj_proc_rig_fpv(i);
			}	
			else {
				obj_proc_rig_tpv(i);
			}

			/*
			 * Apply position-matrix to the rig.
			 */
			mat4_idt(trans_m);
			mat4_pfpos(trans_m, objects.ren_pos[i]);
			rig_mult_mat(objects.rig[i], trans_m);
		}

		if(objects.mask[i] & OBJ_M_MOVE) {
			obj_calc_aim(i);

			obj_update_aim(i);

#if 0
			printf("Position: "); vec3_print(objects.aim_off[i]); printf("\n");
			printf("Direction: "); vec3_print(objects.aim_dir[i]); printf("\n");
#endif
		}
	}
}


extern void obj_sys_render(void)
{
	int i;

	mat4_t idt;
	mat4_t m;

	mat4_idt(idt);
	mat4_idt(m);

	for(i = 0; i < OBJ_LIM; i++) {
		if(objects.mask[i] & OBJ_M_MODEL) {
			mat4_idt(m);

			if((objects.mask[i] & OBJ_M_RIG) == 0) {
				mat4_pfpos(m, objects.ren_pos[i]);
			}

			/* Render the model */
			mdl_render(objects.mdl[i], m, idt, objects.rig[i]);

			if(objects.mask[i] & OBJ_M_MOVE) {
				mat4_t mat;
				vec3_t pos;
				vec3_t dir;

				vec3_cpy(pos, objects.aim_off[i]);
				vec3_scl(objects.aim_dir[i], 4, dir);

				vec3_add(pos, dir, pos);

				/*
				 * Render a sphere at the aiming-point.
				 */
				mat4_idt(mat);
				mat4_pfpos(mat, pos);
				mdl_render(mdl_get("sph"), mat, m, NULL);
			}

			if(objects.mask[i] & OBJ_M_MOVE) {
				mat4_t mat;
				short hook;
				short jnt;

				/* 
				 * Render the weapon.
				 */
			
				hook = handhelds.hook[0];
				jnt = models[objects.mdl[i]]->hok_buf[hook].par_jnt;

				mat4_cpy(mat, objects.rig[i]->tran_mat[jnt]);	
				mat4_idt(m);

				mdl_render(handhelds.mdl[0], mat, m, NULL);
			}
		}
	}
}
