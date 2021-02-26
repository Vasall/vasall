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

	for(i = 0; i < OBJ_SLOTS; i++)
		objects.mask[i] = OBJ_M_NONE;

	objects.num = 0;

	return 0;
}


/* TODO */
extern void obj_close(void)
{
	/* Etto */	
	return;
}


static short obj_get_slot(void)
{
	short i;

	for(i = 0; i < OBJ_SLOTS; i++) {
		if(objects.mask[i] == OBJ_M_NONE)
			return i;
	}

	return -1;
}


static int obj_check_slot(short slot)
{
	if(slot < 0 || slot > OBJ_SLOTS)
		return 1;

	return 0;
}


extern short obj_set(uint32_t id, uint32_t mask, vec3_t pos, short model,
		char *data, int len, uint32_t ts)
{
	short slot;

	/* Get a slot to place object on */
	if((slot = obj_get_slot()) < 0)
		return -1;

	/* Copy the valued and initialize the attributes */
	objects.mask[slot] = mask;
	objects.id[slot] = id;

	/* TODO: Get altitude at current position */
	pos[2] = 0.0;

	/* Setup position, velocity and direction */
	vec3_cpy(objects.pos[slot], pos);
	vec3_clr(objects.vel[slot]);

	/* Initialize object model and rig if requested */
	objects.mdl[slot] = -1;
	if(mask & OBJ_M_MODEL) {
		vec3_set(objects.dir[slot], 0.0, 1.0, 0.0);
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
	mat4_idt(objects.mat_pos[slot]);
	mat4_idt(objects.mat_rot[slot]);

	/* Initialize data-buffer if requested */
	objects.len[slot] = 0;
	if(mask & OBJ_M_DATA) {
		if(data && len) {
			len = (len > OBJ_DATA_MAX) ? (OBJ_DATA_MAX) : (len);
			objects.len[slot] = len;
			memcpy(objects.data[slot], data, len);
		}
	}

	/* Set interpolation variables */
	vec3_cpy(objects.prev_pos[slot], objects.pos[slot]);
	vec3_cpy(objects.prev_dir[slot], objects.dir[slot]);

	/* Setup last input */
	vec3_clr(objects.mov[slot]);
	objects.act[slot] = 0;

	/* Set last update-time */
	objects.last_ack_ts[slot] = ts;
	objects.last_upd_ts[slot] = ts;

	/* Set last acknowledged positio */
	vec3_cpy(objects.last_pos[slot], objects.pos[slot]);

	/* Initialize the object-matrix */
	obj_update_matrix(slot);

	/* Increment number of objects in the object-table */
	objects.num++;
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
		case(OBJ_A_MASK):
			objects.mask[slot] = *(uint32_t *)data;
			break;

		case(OBJ_A_POS):
			vec3_cpy(objects.pos[slot], data);
			break;

		case(OBJ_A_VEL):
			vec3_cpy(objects.vel[slot], data);
			break;

		case(OBJ_A_DIR):
			vec3_cpy(objects.dir[slot], data);
			break;

		case(OBJ_A_BUF):
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

	for(i = 0; i < OBJ_SLOTS; i++) {
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

	for(i = 0; i < OBJ_SLOTS && obj_num < max; i++) {
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

	vec3_cpy(objects.vel[slot], (float *)(ptr + 20));
	vec2_cpy(objects.mov[slot], (float *)(ptr + 32));

	vec3_cpy(objects.last_pos[slot], (float *)(ptr + 8));
	vec3_cpy(objects.last_vel[slot], (float *)(ptr + 20));
	return 0;
}


extern int obj_add_input(short slot, uint32_t mask, uint32_t ts, vec2_t mov,
		uint16_t act)
{		
	short inp_slot;

	if(obj_check_slot(slot))
		return -1;

	/* Get index to place input on */
	inp_slot = objects.inp[slot].num;

	objects.inp[slot].mask[inp_slot] = mask;
	objects.inp[slot].ts[inp_slot] = ts;

	if(mask & INP_M_MOV)
		vec2_cpy(objects.inp[slot].mov[inp_slot], mov);

	if(mask & INP_M_ACT)
		memcpy(&objects.inp[slot].act[inp_slot], &act, 2);

	/* Increment input-counter */
	objects.inp[slot].num++;
	return 0;
}


static int obj_add_inputs(void *in)
{
	uint32_t id;

	short slot;
	short i;
	short num;

	uint32_t mask;
	uint8_t off;
	vec2_t mov;
	uint16_t act = 0;
	uint32_t ts;

	char *ptr = in;
	int read = 0;

	/* Extract timestamp */
	memcpy(&ts, ptr, 4);

	/* Extract general information */
	memcpy(&id,   ptr + 4,  4);
	memcpy(&num,  ptr + 8,  2);

	/* Get the slot of the object */
	if((slot = obj_sel_id(id)) < 0)
		return 0;

	/* Update pointer-position */
	ptr += 10;
	read += 10;

	for(i = 0; i < num; i++) {
		memcpy(&mask, ptr, 4);
		ptr += 4;

		memcpy(&off, ptr, 1);
		ptr += 1;

		if(mask & INP_M_MOV) {
			vec2_cpy(mov, (float *)ptr);
			ptr += VEC2_SIZE;
		}

		/* Update timestamp */
		ts += off;

		/* Add a new input to the object */
		obj_add_input(slot, mask, ts, mov, act);

		read += 13;
	}

	return read;
}

static int obj_set_marker(void *in)
{
	uint32_t ts;
	uint32_t id;
	short slot;
	char *ptr = in;

	memcpy(&ts, ptr, 4);
	ptr += 4;

	/* Skip object number */
	ptr += 2;

	memcpy(&id, ptr, 4);
	ptr += 4;

	printf("id: %u\n", id);

	if((slot = obj_sel_id(id)) < 0)
		return -1;

	printf("Set marker\n");

	objects.mark_flg[slot] = 1;

	memcpy(&objects.mark[slot].ts, &ts, 4);

	vec3_cpy(objects.mark[slot].pos, (float *)ptr);
	ptr += 12;

	vec3_cpy(objects.mark[slot].vel, (float *)ptr);
	ptr += 12;

	vec2_cpy(objects.mark[slot].mov, (float *)ptr);
	ptr += 8;

	return 0;
}

extern int obj_update(void *in)
{
	char *ptr = in;
	uint8_t flg;
	int tmp;

	memcpy(&flg, ptr, 1);
	ptr += 1;

	/* Add inputs */
	if(flg & (1<<0)) {
		if((tmp = obj_add_inputs(ptr)) < 1)
			return -1;

		ptr += tmp;
	}

	/* Set marker */
	if(flg & (1<<1)) {
		if((obj_set_marker(ptr)) < 0)
			return -1;
	}

	return 0;
}


/* Collect all triangles the object collides with */
static void checkCollision(struct col_pck *pck)
{
	int i;
	int j;
	int k;

	struct model *mdl;

	/* Go through all objects */
	for(i = 0; i < OBJ_SLOTS; i++) {
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

			trig_check(pck, vtx[0], vtx[1], vtx[2]);
		}
	}
}

static void collideWithWorld(struct col_pck *pck, vec3_t pos, vec3_t del, int recDepth, vec3_t *opos)
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
	col_create_nrm(&slidingPlane, slidePlaneOrigin, slidePlaneNormal);


	/* Calculate the new destination point */
	tmpDist = col_dist(&slidingPlane, destPoint);	
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
	struct col_pck pck;

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


extern void obj_move(short slot)
{
	uint32_t omask;
	uint32_t mask;
	vec3_t pos;
	vec3_t vel;
	vec3_t acl;
	vec3_t del;
	vec3_t prev;
	vec2_t mov;
	float t_speed = 4.0;

	uint32_t lim_ts;
	uint32_t run_ts;

	short inp_i;
	short inp_num;

	vec3_t grav = {0.0, 0.0, -9.81};


	/* Check if objects is defined */
	if((omask = objects.mask[slot]) == OBJ_M_NONE)
		return;

	/* Check if object is moveable */
	if((omask & OBJ_M_MOVE) == 0)
		return;

	/* Get object-data to current state */
	if(objects.inp[slot].num > 0) {
		vec3_cpy(pos, objects.last_pos[slot]);
		vec3_cpy(vel, objects.last_vel[slot]);

		lim_ts = objects.inp[slot].ts[0];
		run_ts = objects.last_ack_ts[slot];

		/* Drop old inputs */
		if(run_ts > lim_ts)
			return;
	}
	else {
		vec3_cpy(pos, objects.pos[slot]);
		vec3_cpy(vel, objects.vel[slot]);

		lim_ts = core.now_ts;
		run_ts = objects.last_upd_ts[slot];
	}

	vec2_cpy(mov, objects.mov[slot]);
#if 0
	vec3_cpy(dir, objects.dir[slot]);
#endif

	inp_i = -1;
	inp_num = objects.inp[slot].num;

	while(1) {
		while(run_ts < lim_ts) {
			/* 
			 * Process Friction.
			 */
			vec3_scl(vel, (1.0 - TICK_TIME_S * t_speed), vel);

			/*
			 * Process Movement-Acceleration.
			 */
			vec3_set(acl, mov[0], mov[1], 0.0);
			vec3_scl(acl, 6, acl);
			vec3_scl(acl, (TICK_TIME_S * t_speed), acl);

			/* Update velocity of the object */
			vec3_add(vel, acl, vel);

			/*
			 * Process Gravity.
			 */	
			if(omask & OBJ_M_GRAV) {
				vec3_scl(grav, (TICK_TIME_S * t_speed), acl);
			
				/* Update velocity of the object */
				vec3_add(vel, acl, vel);
			}

			/* Save current position */
			vec3_cpy(prev, pos);

			/* Scale velocity by tick-time */
			vec3_scl(vel, TICK_TIME_S, del);
			del[2] = 0.0;

			/* Check collision */
			if(omask & OBJ_M_SOLID) {
				/* Collide and update position */
				collideAndSlide(slot, pos, del, pos);
			}
			else {
				/* Update position */
				vec3_add(pos, del, pos);
			}


			if(omask & OBJ_M_GRAV) {
				/* Scale velocity by tick-time */
				vec3_scl(vel, TICK_TIME_S, del);
				del[0] = 0.0;
				del[1] = 0.0;

				/* Check collision */
				if(omask & OBJ_M_SOLID) {
					/* Collide and update position */
					if(collideAndSlide(slot, pos, del, pos)) {
						vel[2] = 0;
					}
				}
				else {
					/* Update position */
					vec3_add(pos, del, pos);
				}
			}

			/* Limit movement-space */
			if(ABS(pos[0]) > 32.0) {
				pos[0] = 32.0 * SIGN(pos[0]);
				vel[0] = 0;
			}

			if(ABS(pos[1]) > 32.0) {
				pos[1] = 32.0 * SIGN(pos[1]);
				vel[1] = 0;
			}

			if(objects.mark_flg[slot]) {	
				if(run_ts == objects.mark[slot].ts) {
					vec3_cpy(pos, objects.mark[slot].pos);
					vec3_cpy(vel, objects.mark[slot].vel);
					vec2_cpy(mov, objects.mark[slot].mov);
					objects.mark_flg[slot] = 0;
				}
			}

			/* Update run-timer */
			run_ts += TICK_TIME;
		}

		/* If all inputs have been processed */
		inp_i++;
		if(inp_i >= inp_num) {	
			objects.inp[slot].num = 0;

			vec3_cpy(objects.pos[slot], pos);
			vec3_cpy(objects.vel[slot], vel);
			vec2_cpy(objects.mov[slot], mov);

			return;
		}

		mask = objects.inp[slot].mask[inp_i];

		if(mask & INP_M_MOV) {
			vec2_cpy(mov, objects.inp[slot].mov[inp_i]);
		}

		if(inp_i + 1 < inp_num) {
			lim_ts = objects.inp[slot].ts[inp_i + 1];
		}
		else {
			/* Save current status as last acknowledged status */
			vec3_cpy(objects.last_pos[slot], pos);
			vec3_cpy(objects.last_vel[slot], vel);
			objects.last_ack_ts[slot] = objects.inp[slot].ts[inp_i];

			lim_ts = core.now_ts;
		}
	}
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
 * object-systems
 */

extern void obj_sys_update(void)
{
	int i;
	uint32_t ts = core.now_ts;

	for(i = 0; i < OBJ_SLOTS; i++) {
		if(objects.mask[i] & OBJ_M_MOVE) {
			obj_move(i);
		}

		objects.last_upd_ts[i] = ts;
	}
}


extern void obj_sys_prerender(float interp)
{
	int i;
	vec3_t del;

	for(i = 0; i < OBJ_SLOTS; i++) {
		if(objects.mask[i] & OBJ_M_RIG) {
			float agl;
			vec3_t up = {0, 0, 1};

			agl = vec3_angle(up, objects.dir[i]);
			agl = RAD_TO_DEG(agl) - 90.0;

			if(i == core.obj) {
				if(camera.mode == CAM_MODE_FPV) {
					rig_update(objects.rig[i], 0);

					vec3_t off = {0, 0, 1.6};
					vec3_t rev = {0, 0, 1.8};
					vec3_t rot_agl;
					mat4_t rot;

					vec3_set(rot_agl, agl, 0, 0);

					mat4_idt(rot);
					mat4_rfagl(rot, rot_agl);
	
					rig_fpv_rot(objects.rig[i], off, rev, rot);
				}
				else {
					rig_update(objects.rig[i], -agl);
				}
			}
		}

		if(objects.mask[i] & OBJ_M_MODEL) {
			/* Update render-position of the object */
			vec3_sub(objects.pos[i], objects.prev_pos[i], del);
			vec3_scl(del, interp, del);
			vec3_add(objects.prev_pos[i], del, objects.ren_pos[i]);

			/* Store last position */
			vec3_cpy(objects.prev_pos[i], objects.pos[i]);

			/* Update the direction of the object */
			vec3_sub(objects.dir[i], objects.prev_dir[i], del);
			vec3_scl(del, interp, del);
			vec3_add(objects.prev_dir[i], del, objects.ren_dir[i]);

			/* Store last direction */
			vec3_cpy(objects.prev_dir[i], objects.dir[i]);
		}
	}
}


extern void obj_sys_render(void)
{
	int i;
	vec3_t pos;
	vec3_t dir;
	float rot;

	for(i = 0; i < OBJ_SLOTS; i++) {
		if(objects.mask[i] & OBJ_M_MODEL) {
			vec3_cpy(pos, objects.ren_pos[i]);
			vec3_cpy(dir, objects.ren_dir[i]);

			/* Set the position of the model */
			objects.mat_pos[i][0xc] = pos[0];
			objects.mat_pos[i][0xd] = pos[1];
			objects.mat_pos[i][0xe] = pos[2];

			/* Set the rotation of the model */
			rot = atan2(-dir[0], dir[1]);	

			objects.mat_rot[i][0x0] =  cos(rot);
			objects.mat_rot[i][0x1] =  sin(rot);
			objects.mat_rot[i][0x4] =  -sin(rot);
			objects.mat_rot[i][0x5] =  cos(rot);

			mdl_render(objects.mdl[i], objects.mat_pos[i],
					objects.mat_rot[i], objects.rig[i]);
		}
	}
}
