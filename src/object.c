#include "object.h"
#include "world.h"
#include "network.h"

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
		char *data, int len)
{
	short slot;
	float x;
	float z;
	uint32_t ts;

	/* Get current timestamp */
	ts = SDL_GetTicks();

	/* Get slot to place object on */
	if((slot = obj_get_slot()) < 0)
		return -1;

	/* Copy the valued and initialize the attributes */
	objects.mask[slot] = mask;
	objects.id[slot] = id;

	/* Setup position, velocity and direction */
	vec3_cpy(objects.pos[slot], pos);
	vec3_clr(objects.vel[slot]);

	/* Get the height of the terrain at the objects-position */
	x = objects.pos[slot][0];
	z = objects.pos[slot][2];
	objects.pos[slot][1] = wld_get_height(x, z) + 5.6;

	vec3_cpy(objects.prev_pos[slot], objects.pos[slot]);

	/* Setup model-data */
	vec3_set(objects.dir[slot], 1, 0, 1);
	objects.model[slot] = model;
	objects.anim[slot] = 0;
	objects.prog[slot] = 0.0;
	mat4_idt(objects.mat[slot]);

	/* Setup object-data-buffer */
	objects.len[slot] = 0;

	if(data && len) {
		len = (len > OBJ_DATA_MAX) ? (OBJ_DATA_MAX) : (len);
		objects.len[slot] = len;
		memcpy(objects.buf[slot], data, len);
	}

	/* Setup last input */
	vec3_clr(objects.mov[slot]);
	objects.act[slot] = 0;

	/* Set last update-time */
	objects.last_ack_ts[slot] = 0;
	objects.last_upd_ts[slot] = ts;

	/* Set last acknowledged positio */
	vec3_cpy(objects.last_pos[slot], pos);

	/* Initialize the object-matrix */
	obj_update_matrix(slot);

	/* Increment number of objects in the object-table */
	objects.num++;
	return slot;
}


extern void obj_del(short slot)
{
	if(obj_check_slot(slot))
		return;

	objects.mask[slot] = OBJ_M_NONE;

	objects.num--;
}


extern int obj_mod(short slot, short attr, void *data, int len)
{
	if(obj_check_slot(slot) || objects.mask[slot] == OBJ_M_NONE)
		return -1;

	switch(attr) {
		case(OBJ_ATTR_MASK):
			objects.mask[slot] = *(uint32_t *)data;
			break;

		case(OBJ_ATTR_POS):
			vec3_cpy(objects.pos[slot], data);
			break;

		case(OBJ_ATTR_VEL):
			vec3_cpy(objects.vel[slot], data);
			break;

		case(OBJ_ATTR_BUF):
			objects.len[slot] = len;
			memcpy(objects.buf[slot], data, len);
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

	/* Reset the model-matrix to an identity-matrix */
	mat4_idt(objects.mat[slot]);

	/* Set the rotation of the model */
	rot = atan2(-objects.dir[slot][2], objects.dir[slot][0]);
	objects.mat[slot][0x0] =  cos(rot);
	objects.mat[slot][0x2] = -sin(rot);
	objects.mat[slot][0x8] =  sin(rot);
	objects.mat[slot][0xa] =  cos(rot);

	/* Set the position of the model */
	objects.mat[slot][0xc] = objects.pos[slot][0];
	objects.mat[slot][0xd] = objects.pos[slot][1];
	objects.mat[slot][0xe] = objects.pos[slot][2];
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


extern int obj_collect(void *in, short in_num, void **out, short *out_num)
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
			memcpy(ptr, id_ptr, 4);
			ptr += 4;

			/* Copy object-mask */
			memcpy(ptr, &objects.mask[slot], 4);
			ptr += 4;

			/* Copy the position */
			memcpy(ptr, objects.pos[slot], 12);
			ptr += 12;

			/* Copy the velocity of the object */
			memcpy(ptr, objects.vel[slot], 12);
			ptr += 12;

			/* Copy the mov-force of the object */
			vec2_cpy((float *)ptr, objects.mov[slot]);
			ptr += VEC2_SIZE;

			/* Increment the number of objects */
			num++;

			/* Update number of bytes written */
			written += 44;
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


extern int obj_submit(void *in, int64_t ts)
{
	uint32_t id;
	uint32_t mask;
	vec3_t pos;
	short slot;
	short mdl;
	char *ptr = in;

	memcpy(&id,   ptr,       4);
	memcpy(&mask, ptr +  4,  4);
	memcpy(pos,   ptr +  8, 12);

	mdl = mdl_get("plr");

	if((slot = obj_set(id, mask, pos, mdl, NULL, 0)) < 0)
		return -1;

	vec3_cpy(objects.vel[slot], (float *)(ptr + 20));
	vec2_cpy(objects.mov[slot], (float *)(ptr + 32));

	objects.last_ack_ts[slot] = ts;
	objects.last_upd_ts[slot] = ts;

	vec3_cpy(objects.last_vel[slot], (float *)(ptr + 20));

	return 0;
}


extern int obj_add_inputs(uint32_t ts, void *in)
{
	uint32_t id;

	short slot;
	short i;
	short num;

	uint32_t mask;
	uint8_t off;
	vec2_t mov;
	uint16_t act = 0;

	char *ptr;

	/* Extract general information */
	memcpy(&id,   in,  4);
	memcpy(&num,  in + 4,  2);

	/* Get the slot of the object */
	if((slot = obj_sel_id(id)) < 0)
		return -1;

	/* Set pointer */
	ptr = in + 6;

	for(i = 0; i < num; i++) {
		memcpy(&mask, ptr, 4);
		ptr += 4;

		memcpy(&off, ptr, 1);
		ptr += 1;

		if((mask & SHARE_M_MOV) == SHARE_M_MOV) {
			vec2_cpy(mov, (float *)ptr);
			ptr += VEC2_SIZE;
		}

		/* Add a new input to the object */
		obj_add_input(slot, mask, ts, mov, act);

		/* Update timestamp */
		ts += off;
	}
	return 0;
}


extern int obj_add_input(short slot, uint32_t mask, uint32_t ts, vec2_t mov, uint16_t act)
{		
	short inp_slot;

	if(act) {/* Prevent warning for not using parameter */}

	if(obj_check_slot(slot))
		return -1;

	/* Get index to place input on */
	inp_slot = objects.inp[slot].num;

	objects.inp[slot].mask[inp_slot] = mask;
	objects.inp[slot].ts[inp_slot] = ts;

	if(mask & SHARE_M_MOV)
		vec2_cpy(objects.inp[slot].mov[inp_slot], mov);

	/* Increment input-counter */
	objects.inp[slot].num++;
	return 0;
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

extern void obj_sys_input(void)
{
	short i;

	float t_speed = 12.0;

	short inp_i;
	short inp_num;

	uint32_t mask;

	uint32_t lim_ti;
	uint32_t cur_ti;
	uint32_t ti;
	vec3_t pos;
	vec3_t vel;
	vec2_t mov;

	vec3_t prev;
	vec3_t dir;

	vec3_t acl;
	vec3_t del;

	ti = SDL_GetTicks();

	for(i = 0; i < OBJ_SLOTS; i++) {
		if(objects.inp[i].num < 1)
			continue;

		/* Get object-data to current state */	
		vec3_cpy(pos, objects.last_pos[i]);
		vec3_cpy(vel, objects.last_vel[i]);
		vec2_cpy(mov, objects.mov[i]);

		lim_ti = objects.inp[i].ts[0];
		cur_ti = objects.last_ack_ts[i];

		inp_i = -1;
		inp_num = objects.inp[i].num;

		while(1) {
			while(cur_ti < lim_ti) {
				/* Friction */
				vec3_scl(vel, (1.0 - TICK_TIME_S * t_speed), vel);

				/* Acceleration */
				vec3_set(acl, mov[0], 0, mov[1]);
				vec3_scl(acl, 6, acl);
				vec3_scl(acl, (TICK_TIME_S * t_speed), acl);

				/* Update velocity of the player-object */
				vec3_add(vel, acl, vel);

				vec3_scl(vel, TICK_TIME_S, del);

				vec3_cpy(prev, pos);
				vec3_add(pos, del, pos);

				pos[1] = wld_get_height(pos[0], pos[2]) + 5.6;

				if(del[0] + del[2] != 0.0) {
					vec3_nrm(del, dir);
				}

				cur_ti += TICK_TIME;
			}
	
			vec3_cpy(objects.pos[i], pos);
			vec3_cpy(objects.vel[i], vel);
			vec2_cpy(objects.mov[i], mov);
			/* TODO Check if this is better
			 * vec3_cpy(objects.prev_pos[i], prev);*/

			inp_i++;
			if(inp_i >= inp_num) {	
				objects.inp[i].num = 0;

				vec3_cpy(objects.last_pos[i], pos);
				vec3_cpy(objects.last_vel[i], vel);

				objects.last_ack_ts[i] = cur_ti;
				objects.last_upd_ts[i] = cur_ti;
				break;
			}

			mask = objects.inp[i].mask[inp_i];

			if((mask & SHARE_M_MOV) == SHARE_M_MOV)
				vec2_cpy(mov, objects.inp[i].mov[inp_i]);


			if(inp_i + 1 < inp_num)
				lim_ti = objects.inp[i].ts[inp_i + 1];
			else
				lim_ti = ti;
		}
	}
}

extern void obj_sys_update(void)
{
	int i;
	uint32_t ts;

	vec3_t pos;
	vec3_t vel;

	vec3_t acl;
	vec3_t del;

	vec3_t prev;

	float t_speed = 12.0;
	float tmp = TICK_TIME_S * t_speed;

	ts = SDL_GetTicks();

	for(i = 0; i < OBJ_SLOTS; i++) {
		if((objects.mask[i] & OBJ_M_MOVE) == OBJ_M_MOVE) {
			vec3_cpy(pos, objects.pos[i]);
			vec3_cpy(vel, objects.vel[i]);
			vec3_cpy_v2(acl, objects.mov[i]);

			/* Calculate friction */
			vec3_scl(vel, (1.0 - tmp), vel);

			/* Calculate acceleration */
			vec3_scl(acl, 6.0, acl);
			vec3_scl(acl, tmp, acl);

			/* Update velocity of the player-object */
			vec3_add(vel, acl, vel);

			/* Get position-delta and move object */
			vec3_scl(vel, TICK_TIME_S, del);
			vec3_cpy(prev, pos);
			vec3_add(pos, del, pos);

			/* Get height at new position */
			pos[1] = wld_get_height(pos[0], pos[2]) + 5.6;

			vec3_cpy(objects.pos[i], pos);
			vec3_cpy(objects.vel[i], vel);
			vec3_cpy(objects.prev_pos[i], prev);

			/* Adjust direction if moving */
			if(del[0] + del[2] != 0.0) {
				vec3_nrm(del, objects.dir[i]);
			}
		}

		objects.last_upd_ts[i] = ts;
	}
}


extern void obj_sys_render(float interp)
{
	int i;
	vec3_t del;
	vec3_t pos;

	if(interp) {/* Prevent warning for not using parameters */}

	for(i = 0; i < OBJ_SLOTS; i++) {
		if((objects.mask[i] & OBJ_M_MODEL) == OBJ_M_MODEL) {
			vec3_sub(objects.pos[i], objects.prev_pos[i], del);
			vec3_scl(del, interp, del);
			vec3_add(objects.pos[i], del, pos);

			/* Set the position of the model */
			objects.mat[i][0xc] = pos[0];
			objects.mat[i][0xd] = pos[1];
			objects.mat[i][0xe] = pos[2];

			mdl_render(objects.model[i], objects.mat[i]);
		}
	}
}
