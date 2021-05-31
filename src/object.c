#include "object.h"

#include "world.h"
#include "network.h"
#include "collision.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Redefine global object-wrapper */
struct obj_wrapper g_obj;


extern int obj_init(void)
{
	int i;

	for(i = 0; i < OBJ_LIM; i++) {
		g_obj.order[i] = i;
		g_obj.mask[i] = OBJ_M_NONE;
	}

	g_obj.num = 0;
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
		if(g_obj.mask[i] == OBJ_M_NONE)
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
		g_obj.order[i] = i;
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
			a = g_obj.order[i];
			b = g_obj.order[i + 1];

			if(g_obj.mask[a] != 0 && g_obj.mask[b] != 0) {
				if(g_obj.id[a] > g_obj.id[b]) {
					found = 1;

					g_obj.order[i] = b;
					g_obj.order[i + 1] = a;
				}
			}
			else if(g_obj.mask[a] == 0 && g_obj.mask[b] != 0) {
				found = 1;

				g_obj.order[i] = b;
				g_obj.order[i + 1] = a;
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
	g_obj.mask[slot] = mask;
	g_obj.id[slot] = id;

	/* Set the timestamp of the object */
	g_obj.ts[slot] = ts;

	/* Setup position and velocity */
	vec3_cpy(g_obj.pos[slot], pos);
	vec3_clr(g_obj.vel[slot]);

	/* Set movement and direction */
	vec2_clr(g_obj.mov[slot]);
	vec3_set(g_obj.dir[slot], 0, 1, 0);

	/* Initialize interpolation variables */
	g_obj.prev_ts[slot] = g_obj.ts[slot];
	vec3_cpy(g_obj.prev_pos[slot], g_obj.pos[slot]);
	vec3_cpy(g_obj.prev_dir[slot], g_obj.dir[slot]);

	/* Initialize the movement-log */
	obj_log_reset(slot);
	obj_log_set(slot, g_obj.ts[slot], g_obj.pos[slot],
			g_obj.vel[slot], g_obj.mov[slot],
			g_obj.dir[slot]);

	/* Initialize object model and rig if requested */
	g_obj.mdl[slot] = -1;
	if(mask & OBJ_M_MODEL) {
		g_obj.mdl[slot] = model;

		/* Attach a rig to the object */
		g_obj.rig[slot] = NULL;
		if(mask & OBJ_M_RIG) {
			if(models[model]->attr_m & MDL_M_RIG) {
				if(!(g_obj.rig[slot] = rig_derive(model)))
					goto err_reset_slot;
			}
		}
	}

	/* Initialize the position and rotation matrices */
	obj_update_matrix(slot);

	/* Initialize data-buffer if requested */
	g_obj.len[slot] = 0;
	if(mask & OBJ_M_DATA) {
		if(data && len) {
			len = (len > OBJ_DATA_MAX) ? (OBJ_DATA_MAX) : (len);
			g_obj.len[slot] = len;
			memcpy(g_obj.data[slot], data, len);
		}
	}

	/* Initialize the handheld-handle */
	g_obj.hnd[slot].idx = -1;
	if(mask & OBJ_M_MOVE) {
		g_obj.hnd[slot].idx = 0;
	}

	/* Increment number of objects in the object-table */
	g_obj.num++;

	/* Sort the order of objects in the object-list */
	obj_sort();

	/* Sort the objects */
	return slot;

err_reset_slot:
	g_obj.mask[slot] = OBJ_M_NONE;
	return -1;
}


extern void obj_del(short slot)
{
	if(obj_check_slot(slot))
		return;

	/* Delete rig */
	if(g_obj.mask[slot] & OBJ_M_RIG)
		rig_free(g_obj.rig[slot]);

	g_obj.mask[slot] = OBJ_M_NONE;
	g_obj.num--;
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

	g_obj.rig[slot] = rig;	
	return 0;
}


extern int obj_mod(short slot, short attr, void *data, int len)
{
	if(obj_check_slot(slot) || g_obj.mask[slot] == OBJ_M_NONE)
		return -1;

	switch(attr) {
		case OBJ_A_MASK:
			g_obj.mask[slot] = *(uint32_t *)data;
			break;

		case OBJ_A_POS:
			vec3_cpy(g_obj.pos[slot], data);
			break;

		case OBJ_A_VEL:
			vec3_cpy(g_obj.vel[slot], data);
			break;

		case OBJ_A_DIR:
			vec3_cpy(g_obj.dir[slot], data);
			break;

		case OBJ_A_BUF:
			g_obj.len[slot] = len;
			memcpy(g_obj.data[slot], data, len);
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
		if(g_obj.mask[i] == OBJ_M_NONE)
			continue;

		if(g_obj.id[i] == id)
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
	mat4_idt(g_obj.mat_pos[slot]);
	mat4_idt(g_obj.mat_rot[slot]);

	/* Set the position of the model */
	g_obj.mat_pos[slot][0xc] = g_obj.pos[slot][0];
	g_obj.mat_pos[slot][0xd] = g_obj.pos[slot][1];
	g_obj.mat_pos[slot][0xe] = g_obj.pos[slot][2];

	/* Set the rotation of the model */
	rot = atan2(g_obj.dir[slot][1], g_obj.dir[slot][0]);
	g_obj.mat_rot[slot][0x0] =  cos(rot);
	g_obj.mat_rot[slot][0x1] =  sin(rot);
	g_obj.mat_rot[slot][0x4] = -sin(rot);
	g_obj.mat_rot[slot][0x5] =  cos(rot);
}


extern int obj_list(void *ptr, short *num, short max)
{
	short obj_num = 0;
	short i;

	char *buf_ptr = (char *)ptr + 2;

	for(i = 0; i < OBJ_LIM && obj_num < max; i++) {
		if(g_obj.mask[i] == OBJ_M_NONE)
			continue;

		/* Write the id */
		memcpy(buf_ptr, &g_obj.id[i], 4);
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
				memcpy(ptr, &g_obj.mask[slot], 4);
				ptr += 4;
				written += 4;
			}

			/* Copy the position */
			if(flg & OBJ_A_POS) {
				vec3_cpy((float *)ptr, g_obj.pos[slot]);
				ptr += VEC3_SIZE;
				written += VEC3_SIZE;
			}

			/* Copy the velocity of the object */
			if(flg & OBJ_A_VEL) {
				vec3_cpy((float *)ptr, g_obj.vel[slot]);
				ptr += VEC3_SIZE;
				written += VEC3_SIZE;
			}

			/* Copy the mov-force of the object */
			if(flg & OBJ_A_MOV) {
				vec2_cpy((float *)ptr, g_obj.mov[slot]);
				ptr += VEC2_SIZE;
				written += VEC2_SIZE;
			}

			if(flg & OBJ_A_BUF) {
				int tmp = g_obj.len[slot];

				memcpy(ptr, &tmp, 4);
				ptr += 4;
				written += 4;

				memcpy(ptr, g_obj.data[slot], tmp);
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
		if(g_obj.mask[i] == OBJ_M_NONE)
			continue;

		/* Don't check collision with the same object */
		if(i == pck->objSlot)
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

			/* Load triangle vertices and convert to eSpace */
			for(k = 0; k < 3; k++) {
				vec3_t tmpv;

				/* Copy vertex */
				vec3_cpy(tmpv, mdl->col.cm_vtx[idx[k]]);

				/* Move relative to object-position */
				vec3_add(g_obj.pos[i], tmpv, vtx[k]);

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
	vec3_add(pos, models[g_obj.mdl[slot]]->col.ne_col.pos, relpos);

	/*
	 * Set object slot, so the object won't check itself for collision.
	 */
	pck.objSlot = slot;

	/*
	 * Copy the scaling-factors of the sphere, which will be used for
	 * conversion from R3-Space to eSpace.
	 */
	vec3_cpy(pck.eRadius, models[g_obj.mdl[slot]]->col.ne_col.scl);

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
	vec3_sub(opos, models[g_obj.mdl[slot]]->col.ne_col.pos, opos);

	/*
	 * Return if a collision occurred.
	 */
	return retMask;
}


extern void obj_print(short slot)
{
	if(obj_check_slot(slot))
		return;

	printf("Display object %d:\n", slot);
	printf("Pos: "); vec3_print(g_obj.pos[slot]); printf("\n");
	printf("Vel: "); vec3_print(g_obj.vel[slot]); printf("\n");
	printf("Dir: "); vec3_print(g_obj.dir[slot]); printf("\n");
	printf("Mov: "); vec2_print(g_obj.mov[slot]); printf("\n");
}

/*
 * ============================================
 *                OBJECT_SYSTEMS
 * ============================================
 */

static void obj_proc_rig_fpv(short slot)
{
	/* Calculate rig without aiming */
	rig_update(g_obj.rig[slot]);
}

#if 0
static void obj_proc_rig_tpv(short slot, vec3_t pos, vec3_t dir)
{
	float agl;
	vec3_t up = {0, 0, 1};
	vec3_t forw;
	float rot;
	mat4_t pos_m;
	mat4_t rot_m;
	mat4_t trans_m;

	agl = vec3_angle(up, g_obj.dir[slot]);
	agl = 90.0 - RAD_TO_DEG(agl);

	/* Calculate rig with aiming */
	rig_update(g_obj.rig[slot], agl);

	vec2_set(forw, g_obj.dir[slot][0], g_obj.dir[slot][1]);
	vec2_nrm(forw, forw);

	/* Set the rotation of the model */
	rot = RAD_TO_DEG(atan2(-forw[0], forw[1]));

	/* Calculate rotation-matrix */
	mat4_idt(rot_m);
	mat4_rfagl_s(rot_m, 0, 0, rot);

	/* Calculate position-matrix */
	mat4_idt(pos_m);
	mat4_pfpos(pos_m, pos);

	/* Calculate transformation-matrix */
	mat4_mult(pos_m, rot_m, trans_m);

	rig_mult_mat(g_obj.rig[slot], trans_m);
}
#endif

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

	struct inp_entry inp;

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
			if((g_obj.mask[i] & OBJ_M_MOVE) == 0)
				continue;

			if(g_obj.ts[i] > inp_ts) {
				obj_log_cpy(i, logi[i],
						&g_obj.ts[i],
						g_obj.pos[i],
						g_obj.vel[i],
						g_obj.mov[i],
						g_obj.dir[i]);
			}

			/* Update run-ts to the oldest timestamp */
			if(run_ts > g_obj.ts[i]) {
				run_ts = g_obj.ts[i];
			}
		}

		/* Set the run-limit */	
		lim_ts = inp_ts;
	}
	else {
		for(i = 0; i < OBJ_LIM; i++) {
			if((g_obj.mask[i] & OBJ_M_MOVE) == 0)
				continue;

			/* Update run-ts to the oldest timestamp */
			if(run_ts > g_obj.ts[i]) {
				run_ts = g_obj.ts[i];
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
			for(i = 0; i < g_obj.num; i++) {
				o = g_obj.order[i];

				if((g_obj.mask[o] & OBJ_M_MOVE) == 0)
					continue;

				/* 
				 * Skip if the object doesn't have to be updated
				 * yet.
				 */
				if(run_ts < g_obj.ts[o])
					continue;

				/* 
				 * Process friction.
				 */
				f = 1.0 - TICK_TIME_S * t_speed;
				vec3_scl(g_obj.vel[o], f, g_obj.vel[o]);

				/*
				 * Process movement-acceleration.
				 */

				/* Calculate the direction of acceleration */
				vec3_set(frw, g_obj.dir[o][0], g_obj.dir[o][1], 0);
				vec3_nrm(frw, frw);

				vec3_cross(frw, up, rgt);
				vec3_nrm(rgt, rgt);

				vec3_scl(frw, g_obj.mov[o][1], frw);
				vec3_scl(rgt, g_obj.mov[o][0], rgt);

				vec3_add(frw, rgt, acl);
				vec3_nrm(acl, acl);

				/* Scale acceleration */
				vec3_scl(acl, 6, acl);

				f = TICK_TIME_S * t_speed;
				vec3_scl(acl, f, acl);

				/* Update velocity of the object */
				vec3_add(g_obj.vel[o], acl, g_obj.vel[o]);

				/*
				 * Process gravity.
				 */	
				if(g_obj.mask[o] & OBJ_M_GRAV) {
					f = TICK_TIME_S * t_speed;
					vec3_scl(grav, f, acl);

					/* Update velocity of the object */
					vec3_add(g_obj.vel[o], acl, g_obj.vel[o]);
				}

				/* Scale velocity by tick-time */
				vec3_scl(g_obj.vel[o], TICK_TIME_S, del);
				del[2] = 0.0;

				/* Check collision */
				if(g_obj.mask[o] & OBJ_M_SOLID) {
					/* Collide and update position */
					collideAndSlide(o, g_obj.pos[o],
							del, g_obj.pos[o]);
				}
				else {
					/* Update position */
					vec3_add(g_obj.pos[o], del,
							g_obj.pos[o]);
				}


				if(g_obj.mask[o] & OBJ_M_GRAV) {
					/* Scale velocity by tick-time */
					vec3_scl(g_obj.vel[o], TICK_TIME_S, del);
					del[0] = 0.0;
					del[1] = 0.0;

					/* Check collision */
					if(g_obj.mask[o] & OBJ_M_SOLID) {
						/* Collide and update position */
						if(collideAndSlide(o, g_obj.pos[o],
									del, g_obj.pos[o])) {
							g_obj.vel[o][2] = 0;
						}
					}
					else {
						/* Update position */
						vec3_add(g_obj.pos[o], del,
								g_obj.pos[o]);
					}
				}

				/* Limit movement-space */
				if(ABS(g_obj.pos[o][0]) > 32.0) {
					g_obj.pos[o][0] = 32.0 * SIGN(g_obj.pos[o][0]);
					g_obj.vel[o][0] = 0;
				}

				if(ABS(g_obj.pos[o][1]) > 32.0) {
					g_obj.pos[o][1] = 32.0 * SIGN(g_obj.pos[o][1]);
					g_obj.vel[o][1] = 0;
				}

				g_obj.ts[o] += TICK_TIME;

				/*   */
				if((g_obj.ts[o] % OBJ_LOG_TIME) == 0) {
					obj_log_set(o,
							g_obj.ts[o],
							g_obj.pos[o],
							g_obj.vel[o],
							g_obj.mov[o],
							g_obj.dir[o]);


				}
			}

			/*
			 * Update run-timer.
			 */
			run_ts += TICK_TIME;
		}

		if(inp_get(&inp)) {
			short obj_slot = obj_sel_id(inp.obj_id);

			if(inp.ts >= g_obj.ts[obj_slot]) {
				if(inp.mask & INP_M_MOV)
					vec2_cpy(g_obj.mov[obj_slot], inp.mov);

				if(inp.mask & INP_M_DIR)
					vec3_cpy(g_obj.dir[obj_slot], inp.dir);
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

	for(i = 0; i < OBJ_LIM; i++) {
		if(g_obj.mask[i] & OBJ_M_MOVE) {
			/* Calculate position the object is looking at */
			obj_calc_view(i);
		}

		if(g_obj.mask[i] & OBJ_M_RIG) {
			rig_prepare(g_obj.rig[i]);

			if(i == g_core.obj && g_cam.mode == CAM_MODE_FPV) {
				obj_proc_rig_fpv(i);
			}
			else {
				vec3_t off = {0, 0, 1.8};
				vec3_t pos;
				vec3_t tmp;

				vec4_t calc;
				mat4_t mat;

				/*
				 * Calculate the aim-point relative to the object.
				 */
				vec3_cpy(calc, g_obj.dir[i]);
				calc[3] = 1;
				mat4_inv(mat, g_obj.rot_mat[i]);
				vec4_trans(calc, mat, calc);

				vec3_scl(calc, 10, tmp);
				vec3_add(off, tmp, pos);

				/* Calculate rig with aiming */
				rig_update_aim(g_obj.rig[i], pos);
			}

			rig_finish(g_obj.rig[i]);
		}



		/*
		 * Calculate position and rotation-matrix.
		 */
		if(g_obj.mask[i] & OBJ_M_MOVE) {
			vec3_t forw;
			float rot;
			mat4_t pos_m;
			mat4_t rot_m;

			vec2_set(forw, g_obj.dir[i][0], g_obj.dir[i][1]);
			vec2_nrm(forw, forw);

			/* Set the rotation of the model */
			rot = RAD_TO_DEG(atan2(forw[0], forw[1]));

			/* Calculate rotation-matrix */
			mat4_idt(rot_m);
			if(i != g_core.obj || g_cam.mode != CAM_MODE_FPV) {
				mat4_rfagl_s(rot_m, 0, 0, rot);
			}
			else {
				vec3_t off_v = {0, 0, -1.7};
				vec3_t rev_v = {0, 0, 1.8};

				mat4_t off_m;
				mat4_t rev_m;
				mat4_t pos_m;
				mat4_t trans_m;

				/* 
	 			 * Calculate transformation-matrix for
	 			 * first-person-view.
				 */
				mat4_idt(off_m);
				mat4_idt(rev_m);
				mat4_pfpos(off_m, off_v);
				mat4_pfpos(rev_m, rev_v);

				/* Calculate rotation-matrix */
				mat4_cpy(rot_m, g_cam.forw_m);
				mat4_mult(rot_m, off_m, rot_m);
				mat4_mult(rev_m, rot_m, rot_m);
			}

			/* Calculate position-matrix */
			mat4_idt(pos_m);
			mat4_pfpos(pos_m, g_obj.pos[i]);

			/* Copy matrices to object */
			mat4_cpy(g_obj.pos_mat[i], pos_m);
			mat4_cpy(g_obj.rot_mat[i], rot_m);
		}
		else {
			/* Set position-matrix */
			mat4_idt(g_obj.pos_mat[i]);
			mat4_pfpos(g_obj.pos_mat[i], g_obj.pos[i]);

			/* Set rotation-matrix */
			mat4_idt(g_obj.rot_mat[i]);
		}
	}
}


extern void obj_sys_render(void)
{
	int i;

	mat4_t idt;
	mat4_t rot_m;
	mat4_t pos_m;

	vec4_t calc;
	mat4_t mat;

	mat4_idt(idt);

	for(i = 0; i < OBJ_LIM; i++) {
		if(g_obj.mask[i] & OBJ_M_MODEL) {
			mat4_cpy(pos_m, g_obj.pos_mat[i]);
			mat4_cpy(rot_m, g_obj.rot_mat[i]);

			/* Render the model */
			if((g_obj.mask[i] & OBJ_M_MOVE) == 0 || 1)
				mdl_render(g_obj.mdl[i], pos_m, rot_m, g_obj.rig[i]);

			if(g_obj.mask[i] & OBJ_M_MOVE) {
				/* Calculate position-matrix of hook */
				mat4_idt(pos_m);
				mat4_pfpos(pos_m, g_obj.pos[i]);

				/* Calculate rotation-matrix of hook */
				mat4_cpy(mat, g_hnd.hook_mat[0][0]);
				mat4_mult(g_obj.rig[i]->hook_base_mat[0], mat, mat);
				mat4_mult(g_obj.rot_mat[i], mat, rot_m);

				mdl_render(mdl_get("pistol"), pos_m, rot_m, NULL);
			}

			if(g_obj.mask[i] & OBJ_M_MOVE) {
				vec3_t pos;

				/* Adjust the rotation of the handheld */
				vec3_sub(g_obj.view_pos_rel[i], g_hnd.brl_off[0], pos);

				mat4_idt(pos_m);
				mat4_pfpos(pos_m, g_obj.pos[i]);

				mat4_idt(rot_m);
				mat4_pfpos(rot_m, pos);
				mat4_mult(g_obj.rot_mat[i], rot_m, rot_m);

				mdl_render(mdl_get("sph2"), pos_m, rot_m, NULL);
			}

			if(g_obj.mask[i] & OBJ_M_MOVE) {
				/*
				 * Render a sphere at the aiming-point.
				 */		
				mat4_idt(pos_m);
				mat4_pfpos(pos_m, g_obj.view_pos[i]);
				mdl_render(mdl_get("sph1"), pos_m, idt, NULL);
			}
		}
	}
}
