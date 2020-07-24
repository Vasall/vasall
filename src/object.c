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
	vec3_clr(objects.mov[slot]);

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

	/* Get the height of the terrain at the objects-position */
	x = objects.pos[slot][0];
	z = objects.pos[slot][2];
	objects.pos[slot][1] = wld_get_height(x, z) + 5.6;

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
	uint32_t ts;

	/* Get the current timestamp */
	ts = SDL_GetTicks();

	/* Allocate memory for the return-buffer */
	if(!(out_buf = malloc(in_num * 44 + 6)))
		return 0;

	/* Set the starting-pointer to write the object-data */
	ptr = out_buf + 6;

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
	memcpy(out_buf + 2, &ts, 4);

	/* Set return-variables */
	*out = out_buf;
	if(out_num) *out_num = num;

	/* Return the number of written bytes */
	return written + 6;
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


extern int obj_update(void *in)
{
	uint32_t id;
	short slot;
	uint32_t ti;
	short num;
	short i;
	char *ptr;

	uint32_t mask;
	uint8_t off;

	float t_speed = 12.0;

	uint32_t cur_ti;
	vec3_t pos;
	vec3_t vel;
	vec2_t mov;
	vec3_t dir;
		
	vec3_t acl;
	vec3_t del;

	/* Extract general information */
	memcpy(&ti, in, 4);
	memcpy(&id, in + 4, 4);
	memcpy(&num, in + 8, 2);

	/* Set pointer */
	ptr = in + 10;

	/* Get the slot of the object */
	if((slot = obj_sel_id(id)) < 0)
		return -1;

	/* Get object-data to current state */
	cur_ti = objects.last_ack_ts[slot];	
	vec3_cpy(pos, objects.last_pos[slot]);
	vec3_cpy(vel, objects.last_vel[slot]);
	vec2_cpy(mov, objects.mov[slot]);

	for(i = 0; i < num; i++) {
		while(cur_ti < ti) {
			/* Friction */
			vec3_scl(vel, (1.0 - TICK_TIME_S * t_speed), vel);

			/* Acceleration */
			vec3_set(acl, mov[0], 0, mov[1]);
			vec3_scl(acl, 6, acl);
			vec3_scl(acl, (TICK_TIME_S * t_speed), acl);

			/* Update velocity of the player-object */
			vec3_add(vel, acl, vel);

			vec3_scl(vel, TICK_TIME_S, del);
			vec3_add(pos, del, pos);

			pos[1] = wld_get_height(pos[0], pos[2]) + 5.6;
		
			if(del[0] + del[2] != 0.0) {
				vec3_nrm(del, dir);
			}

			cur_ti += TICK_TIME;
		}

		memcpy(&mask, ptr, 4);
		ptr += 4;

		memcpy(&off, ptr, 1);
		ptr += 1;

		if((mask & SHARE_M_MOV) == SHARE_M_MOV) {
			vec2_cpy(mov, (float *)ptr);
			ptr += VEC2_SIZE;
		}

		ti += off;
	}

	vec3_cpy(objects.pos[slot], pos);
	vec3_cpy(objects.vel[slot], vel);
	vec2_cpy(objects.mov[slot], mov);
	vec3_cpy(objects.dir[slot], dir);

	objects.last_ack_ts[slot] = ti;
	vec3_cpy(objects.last_pos[slot], pos);
	vec3_cpy(objects.last_vel[slot], vel);
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


extern void obj_sys_update(float delt)
{
	vec3_t pos;
	vec3_t del;
	vec3_t v_old;
	vec3_t acl;
	vec3_t vel;
	int i;
	float t_speed = 12.0;

	for(i = 0; i < OBJ_SLOTS; i++) {
		if((objects.mask[i] & OBJ_M_MOVE) == OBJ_M_MOVE) {
			float x, z;

			/* Friction */
			vec3_cpy(v_old, objects.vel[i]);
			vec3_scl(v_old, (1.0 - TICK_TIME_S * t_speed), v_old);

			/* Acceleration */
			vec3_set(vel, objects.mov[i][0], 0, objects.mov[i][1]);
			vec3_scl(vel, 6.0, vel);
			vec3_scl(vel, (TICK_TIME_S * t_speed), acl);

			/* Update velocity of the player-object */
			vec3_add(v_old, acl, objects.vel[i]);	

			vec3_cpy(pos, objects.pos[i]);
			vec3_scl(objects.vel[i], delt, del);
			vec3_add(pos, del, objects.pos[i]);

			x = objects.pos[i][0];
			z = objects.pos[i][2];
			objects.pos[i][1] = wld_get_height(x, z) + 5.6;

			if(vec3_mag(del) > 0.0) {
				vec3_nrm(del, objects.dir[i]);
				obj_update_matrix(i);
			}
		}
	}
}


extern void obj_sys_render(float interp)
{
	int i;

	if(interp) {/* Prevent warning for not using parameters */}

	for(i = 0; i < OBJ_SLOTS; i++) {
		if((objects.mask[i] & OBJ_M_MODEL) == OBJ_M_MODEL) {
			mdl_render(objects.model[i], objects.mat[i]);
		}
	}
}
