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
	objects.model[slot] = -1;
	if(mask & OBJ_M_MODEL) {
		vec3_set(objects.dir[slot], 0.0, 1.0, 0.0);
		objects.model[slot] = model;

		/* Attach a rig to the object */
		objects.rig[slot] = NULL;
		if(mask & OBJ_M_RIG) {
			if(models[model]->type >= MDL_RIG) {
				if(!(objects.rig[slot] = rig_derive(model)))
					goto err_reset_slot;
			}
		}
	}

	objects.col[slot].mask = COL_M_NONE;
	if(mask & OBJ_M_SOLID) {
		/* Copy collision-mask */
		objects.col[slot].mask = models[model]->col_mask;

		if(models[model]->col_mask & COL_M_BP) {
			vec3_t tmp;
			vec3_t relp;

			objects.col[slot].box = models[model]->col.bp_col;

			/* TODO: Quick collision-box update */
			vec3_add(objects.col[slot].box.pos, objects.pos[slot], relp);

			vec3_sub(relp, objects.col[slot].box.scl, objects.col[slot].min);
			vec3_add(relp, objects.col[slot].box.scl, objects.col[slot].max);
		}
	}

#if 0
	/* Initialize collision-buffers if requested */
	object.col[slot].mask = OBJ_COLM_NONE;
	if(mask & OBJ_M_SOLID) {
		/* Add broadphase-collision if possible */
		if(models[model].col_mask & MDL_COLM_BP) {
			int tmp;
			cube_t box = models[model].col.bpcol;

			/* Add the collision-box to the collision-system */
			if(wld_col_add(slot, pos, box, &tmp) < 0)
				goto err_reset_slot;

			/* Update the collision-mask */
			objects.col[slot].mask |= OBJ_COLM_BP;

			/* Set the index of the bp-collider */
			objects.col[slot].bpcol = tmp;
		}
	}
#endif

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
	objects.mat_rot[slot][0x4] =  -sin(rot);
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


extern void obj_move(short slot)
{
	uint32_t mask;
	vec3_t pos;
	vec3_t vel;
	vec3_t acl;
	vec3_t del;
	vec3_t prev;
	vec2_t mov;
	vec3_t dir;
	float t_speed = 4.0;

	uint32_t lim_ts;
	uint32_t run_ts;

	short inp_i;
	short inp_num;


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
	vec3_cpy(dir, objects.dir[slot]);


	inp_i = -1;
	inp_num = objects.inp[slot].num;

	while(1) {
		while(run_ts < lim_ts) {
			/* Friction */
			vec3_scl(vel, (1.0 - TICK_TIME_S * t_speed), vel);

			/* Acceleration */
			vec3_set(acl, mov[0], mov[1], 0.0);
			vec3_scl(acl, 6, acl);
			vec3_scl(acl, (TICK_TIME_S * t_speed), acl);

			/* Update velocity of the player-object */
			vec3_add(vel, acl, vel);

			vec3_scl(vel, TICK_TIME_S, del);

			vec3_cpy(prev, pos);
			vec3_add(pos, del, pos);

			/* Limit movement-space */
			if(ABS(pos[0]) > 32.0) {
				pos[0] = 32.0 * SIGN(pos[0]);
				vel[0] = 0;
			}

			if(ABS(pos[1]) > 32.0) {
				pos[1] = 32.0 * SIGN(pos[1]);
				vel[1] = 0;
			}

			vec3_nrm(vel, dir);

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

		inp_i++;
		if(inp_i >= inp_num) {	
			objects.inp[slot].num = 0;

			vec3_cpy(objects.pos[slot], pos);
			vec3_cpy(objects.vel[slot], vel);
			vec2_cpy(objects.mov[slot], mov);
			vec3_cpy(objects.dir[slot], dir);
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

static int _box_overlap(short a, short b)
{
	static c = 0;

	vec3_t min1;
	vec3_t max1;
	vec3_t min2;
	vec3_t max2;

	vec3_cpy(min1, objects.col[a].min);
	vec3_cpy(max1, objects.col[a].max);
	vec3_cpy(min2, objects.col[b].min);
	vec3_cpy(max2, objects.col[b].max);

	if(c == 0) {
		c = 1;
		printf("min:\n");
		vec3_print(min1); printf("\n");
		vec3_print(min2); printf("\n");

		printf("max:\n");
		vec3_print(max1); printf("\n");
		vec3_print(max2); printf("\n");
	}

	return (min1[0] <= max2[0] && max1[0] >= min2[0]) &&
		(min1[1] <= max2[1] && max1[1] >= min2[1]) &&
		(min1[2] <= max2[2] && max1[2] >= min2[2]);
}

static int _pnt_in_trig(vec3_t pnt, vec3_t pa, vec3_t pb, vec3_t pc)
{
	vec3_t v0;
	vec3_t v1;
	vec3_t v2;

	float d00;
	float d01;
	float d02;
	float d11;
	float d12;

	float invDenom;
	float u;
	float v;

	vec3_sub(pc, pa, v0);
	vec3_sub(pb, pa, v1);
	vec3_sub(pnt, pa, v2);

	d00 = vec3_dot(v0, v0);
	d01 = vec3_dot(v0, v1);
	d02 = vec3_dot(v0, v2);
	d11 = vec3_dot(v1, v1);
	d12 = vec3_dot(v1, v2);

	invDenom = 1 / (d00 * d11 - d01 * d01);
	u = (d11 * d02 - d01 * d12) * invDenom;
	v = (d00 * d12 - d01 * d02) * invDenom;

	return (u >= 0) && (v >= 0) && (u + v < 1);
}

static int _min_root(float a, float b, float c, float maxR,float* root)
{
	/* Check if a solution exists */
	float determinant = b * b - 4.0f * a * c;

	/* If determinant is negative it means no solutions. */ 
	if(determinant < 0.0f)
		return 0;

	/* 
	 * Calculate the two roots: (if determinant == 0 then
	 * x1==x2 but let’s disregard that slight optimization)
	 */
	float sqrtD = SQRT(determinant);

	float r1 = (-b - sqrtD) / (2 * a);
	float r2 = (-b + sqrtD) / (2 * a);

	/* Sort so x1 <= x2 */
	if(r1 > r2) {
		float tmp = r2;
		r2 = r1;
		r1 = tmp;
	}

	/* Get lowest root: */
	if(r1 > 0 && r1 < maxR) {
		*root = r1;
		return 1;
	}

	/* It is possible that we want x2 - this can happen if x1 < 0 */
	if(r2 > 0 && r2 < maxR) {
		*root = r2;
		return 1;
	}

	/* No (valid) solutions */
	return 0;
}

/* Check if a collision between the sphere and triangle occurrs */
static int _trig_check(struct col_pck *pck, vec3_t p0, vec3_t p1,
		vec3_t p2, float *tout, vec3_t vout)
{
	struct col_pln pln;

	int3_t idx;
	vec3_t nrm;
	vec4_t equ;
	float distToPlane;
	float nrmDotVel;

	vec3_t tmp0;
	vec3_t tmp1;

	char embeddedInPlane = 0;
	vec3_t collisionPoint;
	char foundCollision = 0;

	float t = 1.0;

	float t0;
	float t1;

	/* Construct plane */
	col_create_pnt(&pln, p0, p1, p2);

	/* Calculate the signed distance */
	distToPlane = col_dist(&pln, pck->basePoint);

	/* Calculate denominator */
	nrmDotVel = vec3_dot(pln.normal, pck->velocity);

	printf("%f - %f\n", distToPlane, nrmDotVel);

	/* If sphere is traveling parallel to the plane */
	if(nrmDotVel == 0.0) {
		if(ABS(distToPlane) >= 1.0) {
			/* 
			 * Sphere is not embedded in plane.
			 * No collision possible!
			 */
			return 0;
		}
		else {
			/*
			 * Sphere is embedded in plane.
			 * It intresects in the whole range [0..1]
			 */
			embeddedInPlane = 1;
			t0 = 0.0;
			t1 = 1.0;
		}
	}
	else {
		/* N dot D is not 0. Calculate intersection interval: */
		t0 = (-1.0 - distToPlane) / nrmDotVel;
		t1 = ( 1.0 - distToPlane) / nrmDotVel;

		/* Swap so t0 < t1 */
		if(t0 > t1) {
			float swp = t1;
			t1 = t0;
			t0 = swp;
		}

		/* Check that atleast one result is in range */
		if(t0 > 1.0 || t1 < 0.0) {
			/* 
			 * Both t values are outside values [0,1]
			 * No collision possible!
			 */
			return 0;
		}

		/* Clamp to [0, 1] */
		t0 = clamp(t0);
		t1 = clamp(t1);
	}


	/*
	 * OK, at this point we have two time values t0 and t1 between which the
	 * swept sphere intersects with the triangle plane. If any collision is
	 * to occur it must// happen within this interval.
	 */
	t = 1.0;
	foundCollision = 0;

	/* 
	 * First we check for the easy case - collision inside the
	 * triangle. If this happens it must be at time t0 as this is
	 * when the sphere rests on the front side of the triangle
	 * plane. Note, this can only happen if the sphere is not
	 * embedded in the triangle plane.
	 */
	if(!embeddedInPlane) {
		vec3_t interPnt;

		/* Calculate intersection-point */
		vec3_sub(pck->basePoint, pln.normal, tmp0);
		vec3_scl(pck->velocity, t0, tmp1);
		vec3_add(tmp0, tmp1, interPnt);

		/* Intersection-point in triangle */
		if(_pnt_in_trig(interPnt, p0, p1, p2)) {
			foundCollision = 1;
			t = t0;
			vec3_cpy(collisionPoint, interPnt);

			printf("Intersect with triangle\n");
		}
	}

	/*
	 * If we haven’t found a collision already we’ll have to sweep
	 * sphere against points and edges  of the triangle. Note: A
	 * collision inside the triangle (the check above) will always
	 * happen before a vertex or edge collision! This is why we can
	 * skip the swept test if the above// gives a collision!
	 */
	if(foundCollision == 0) {
		float a;
		float b;
		float c;
		float newT;
		vec3_t velocity;
		vec3_t base;
		float velSqrLen;

		vec3_t edge;
		vec3_t posToVtx;
		float edgeSqr;
		float edgeDotVel;
		float edgeDotPosToVtx;
		float velDot;

		/*
		 * Set variables for checking points.
		 */
		vec3_cpy(velocity, pck->velocity);
		vec3_cpy(base, pck->basePoint);
		velSqrLen = vec3_sqrlen(velocity);

		/*
		 * For each vertex or edge a quadratic equation have to
		 * be solved. We parameterize this equation as 
		 * a * t^2 + b * t + c = 0 and below we calculate the 
		 * parameters a,b and c for each test.
		 */

		/*
		 * Check Points
		 */

		a = velSqrLen;

		/* Point 1 */
		vec3_sub(base, p0, tmp0);
		b = 2.0 * vec3_dot(velocity, tmp0);
		vec3_sub(p0, base, tmp0);
		c = vec3_sqrlen(tmp0) - 1.0;

		if(_min_root(a, b, c, t, &newT)) {
			foundCollision = 1;
			t = newT;
			vec3_cpy(collisionPoint, p0);
			printf("With point 0\n");
		}

		/* Point 1 */
		vec3_sub(base, p1, tmp0);
		b = 2.0 * vec3_dot(velocity, tmp0);
		vec3_sub(p1, base, tmp0);
		c = vec3_sqrlen(tmp0) - 1.0;

		if(_min_root(a, b, c, t, &newT)) {
			foundCollision = 1;
			t = newT;
			vec3_cpy(collisionPoint, p1);
			printf("With point 1\n");
		}

		/* Point 2 */
		vec3_sub(base, p2, tmp0);
		b = 2.0 * vec3_dot(velocity, tmp0);
		vec3_sub(p2, base, tmp0);
		c = vec3_sqrlen(tmp0) - 1.0;

		if(_min_root(a, b, c, t, &newT)) {
			foundCollision = 1;
			t = newT;
			vec3_cpy(collisionPoint, p2);
			printf("With point 2\n");
		}

		/*
		 * Check edges
		 */

		velDot = vec3_dot(velocity, posToVtx);

		/* v0 --> v1 */
		vec3_sub(p1, p0, edge);
		vec3_sub(p0, base, posToVtx);
		edgeSqr = vec3_sqrlen(edge);
		edgeDotVel = vec3_dot(edge, velocity);
		edgeDotPosToVtx = vec3_dot(edge, posToVtx);

		a = edgeSqr * -velSqrLen + edgeDotVel * edgeDotVel;
		b = edgeSqr * (2.0 * velDot) - 2.0 * edgeDotVel * edgeDotPosToVtx;
		c = edgeSqr * (1.0 - vec3_sqrlen(posToVtx)) + edgeDotPosToVtx * edgeDotPosToVtx;

		if(_min_root(a, b, c, t, &newT)) {
			/* Check if intersection within line segment */
			float f = (edgeDotVel * newT - edgeDotPosToVtx) / edgeSqr;

			if(f >= 0.0 && f <= 1.0) {
				/* Intersection took place in segment */
				foundCollision = 1;
				t = newT;

				vec3_scl(edge, f, tmp0);
				vec3_add(p0, tmp0, collisionPoint);

				printf("With edge 0\n");
			}
		}

		/* v1 --> v2 */
		vec3_sub(p2, p1, edge);
		vec3_sub(p1, base, posToVtx);
		edgeSqr = vec3_sqrlen(edge);
		edgeDotVel = vec3_dot(edge, velocity);
		edgeDotPosToVtx = vec3_dot(edge, posToVtx);

		a = edgeSqr * -velSqrLen + edgeDotVel * edgeDotVel;
		b = edgeSqr * (2.0 * velDot) - 2.0 * edgeDotVel * edgeDotPosToVtx;
		c = edgeSqr * (1.0 - vec3_sqrlen(posToVtx)) + edgeDotPosToVtx * edgeDotPosToVtx;

		if(_min_root(a, b, c, t, &newT)) {
			/* Check if intersection within line segment */
			float f = (edgeDotVel * newT - edgeDotPosToVtx) / edgeSqr;

			if(f >= 0.0 && f <= 1.0) {
				/* Intersection took place in segment */
				foundCollision = 1;
				t = newT;

				vec3_scl(edge, f, tmp0);
				vec3_add(p1, tmp0, collisionPoint);

				printf("With edge 0\n");
			}
		}

		/* v2 --> v0 */
		vec3_sub(p0, p2, edge);
		vec3_sub(p2, base, posToVtx);
		edgeSqr = vec3_sqrlen(edge);
		edgeDotVel = vec3_dot(edge, velocity);
		edgeDotPosToVtx = vec3_dot(edge, posToVtx);

		a = edgeSqr * -velSqrLen + edgeDotVel * edgeDotVel;
		b = edgeSqr * (2.0 * velDot) - 2.0 * edgeDotVel * edgeDotPosToVtx;
		c = edgeSqr * (1.0 - vec3_sqrlen(posToVtx)) + edgeDotPosToVtx * edgeDotPosToVtx;

		if(_min_root(a, b, c, t, &newT)) {
			/* Check if intersection within line segment */
			float f = (edgeDotVel * newT - edgeDotPosToVtx) / edgeSqr;

			if(f >= 0.0 && f <= 1.0) {
				/* Intersection took place in segment */
				foundCollision = 1;
				t = newT;

				vec3_scl(edge, f, tmp0);
				vec3_add(p2, tmp0, collisionPoint);

				printf("With edge 0\n");
			}
		}
	}

	if(foundCollision) {
		*tout = t;
		vec3_cpy(vout, collisionPoint);

		return 1;
	}

	return 0;
}

/* a is a moveable object and b is either movable or static */
static void obj_hdl_col(short a, short b)
{
	struct model *mdl_a;
	struct model *mdl_b;
	struct col_pck colpck;

	int i;
	int j;

	float t;
	vec3_t tmppnt;
	vec3_t tmp;

	float mint = 2.0;
	vec3_t colpnt;

	/* Get pointers to the models */
	mdl_a = models[objects.model[a]];
	mdl_b = models[objects.model[b]];

	/* Create the collision-package */
	vec3_add(objects.pos[a], mdl_a->col.ne_col.pos, tmp);
	col_init_pck(&colpck, tmp, objects.vel[a], mdl_a->col.ne_col.scl);

	for(i = 0; i < mdl_b->col.cm_tri_c; i++) {
		vec3_t vtx[3];
		int3_t idx;

		/* Get indices of triangles */
		memcpy(idx, mdl_b->col.cm_idx[i], INT3_SIZE);

		/* Load triangle vertices and convert to eSpace */
		for(j = 0; j < 3; j++) {
			/* Copy vertex */
			vec3_add(objects.pos[b], mdl_b->col.cm_vtx[idx[j]], vtx[j]);

			/* Transform to e-space */
			vec3_div(vtx[j], colpck.eRadius, vtx[j]);
		}

		/* Check if a collision occurrs */
		if(_trig_check(&colpck, vtx[0], vtx[1], vtx[2], &t, tmppnt)) {
			printf("collision\n");
			if(t < mint) {
				mint = t;
				vec3_cpy(colpnt, tmppnt);		
			}
		}

	}
}

extern void obj_sys_update(void)
{
	int i;
	int j;
	uint32_t ts = core.now_ts;

	for(i = 0; i < OBJ_SLOTS; i++) {
		if(objects.mask[i] & OBJ_M_MOVE) {
			vec3_t tmp;
			vec3_t relp;

			obj_move(i);

			/* TODO: Quick collision-box update */
			vec3_add(objects.col[i].box.pos, objects.pos[i], relp);

			vec3_sub(relp, objects.col[i].box.scl, objects.col[i].min);
			vec3_add(relp, objects.col[i].box.scl, objects.col[i].max);
		}

		objects.last_upd_ts[i] = ts;
	}

	for(i = 0; i < OBJ_SLOTS; i++) {
		uint32_t mask = objects.mask[i];
		/* TODO: Quick collision */
		if((mask & OBJ_M_SOLID) && (mask & OBJ_M_MOVE)) {
			for(j = 0; j < OBJ_SLOTS; j++) {
				if(i != j && (objects.mask[j] & OBJ_M_SOLID)) {
					if(_box_overlap(i, j)) {
						obj_hdl_col(i, j);
					}
				}
			}
		}
	}
}


extern void obj_sys_prerender(float interp)
{
	int i;
	vec3_t del;

	for(i = 0; i < OBJ_SLOTS; i++) {
		if(objects.mask[i] & OBJ_M_RIG) {
			rig_update(objects.rig[i]);
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

			mdl_render(objects.model[i], objects.mat_pos[i],
					objects.mat_rot[i], objects.rig[i]);
		}
	}
}
