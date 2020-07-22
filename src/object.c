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

	if((slot = obj_get_slot()) < 0)
		return -1;

	/* Copy the valued and initialize the attributes */
	objects.mask[slot] = mask;
	objects.id[slot] = id;
	vec3_cpy(objects.pos[slot], pos);
	vec3_set(objects.vel[slot], 0, 0, 0);
	vec3_set(objects.dir[slot], 1, 0, 1);
	objects.model[slot] = model;
	objects.anim[slot] = 0;
	objects.prog[slot] = 0.0;
	mat4_idt(objects.mat[slot]);

	objects.len[slot] = 0;

	if(data && len) {
		len = (len > OBJ_DATA_MAX) ? (OBJ_DATA_MAX) : (len);
		objects.len[slot] = len;
		memcpy(objects.buf[slot], data, len);
	}

	/* Get the height of the terrain at the objects-position */
	x = objects.pos[slot][0];
	z = objects.pos[slot][2];
	objects.pos[slot][1] = wld_get_height(x, z) + 2.2;

	/* Set last update-time */
	objects.last[slot] = ts;

	/* Initialize the object-matrix */
	obj_update_matrix(slot);

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

		printf("list %d\n", objects.id[i]);

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
		printf("Check %u\n", *id_ptr);
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

			/* Copy the acceleration of the object */
			memcpy(ptr, objects.mov[slot], 12);
			ptr += 12;

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

	printf("Collected %d\n", num);

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

	memcpy(objects.vel[slot], ptr + 20, 12);
	memcpy(objects.mov[slot], ptr + 32, 12);
	objects.last[slot] = ts;

	printf("Added object %d at slot %d\n", id, slot);
	return 0;
}


extern int obj_collect_updates(void **out, short *out_num)
{
	short i;
	short num = 0;
	char *buf;
	char *ptr;
	int written = 0;

	if(!(buf = malloc(objects.num * 40)))
		return -1;

	ptr = buf;

	for(i = 0; i < objects.num; i++) {
		if(objects.mask[i] == OBJ_M_NONE)
			continue;

		memcpy(ptr, &objects.id[i], 4);
		memcpy(ptr + 4, objects.pos, 12);
		memcpy(ptr + 16, objects.vel, 12);
		memcpy(ptr + 28, objects.mov, 12);


		ptr += 40;
		written += 40;
		num++;
	}

	*out = buf;
	*out_num = num;
	return written;
}


extern int obj_update(void *in, short num)
{
	uint32_t id;
	short i;
	char *ptr = in;
	short slot;

	for(i = 0; i < num; i++) {
		memcpy(&id, ptr, 4);

		if(id == network.id)
			continue;

		if((slot = obj_sel_id(id)) >= 0) {
			memcpy(objects.pos[slot], ptr +  4, 12);
			memcpy(objects.vel[slot], ptr + 16, 12);
				memcpy(objects.mov[slot], ptr + 28, 12);
		}

		ptr += 40;
	}

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
	printf("Mov: "); vec3_print(objects.mov[slot]); printf("\n");
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


			/*
			 * Control the speed and direction of the character.
			 *
			 * v_new = v_old * (1 - del_t * t_speed) + v_end * (del_t * t_speed)
			 * v_new - v_end * (del_t * t_speed) = v_old * (1 - del_t * t_speed)
			 * v_old = (v_new - v_end * (del_t * t_speed)) / (1 - del_t * t_speed)
			 */

			/* Friction */
			vec3_cpy(v_old, objects.vel[i]);
			vec3_scl(v_old, (1.0 - TICK_TIME_S * t_speed), v_old);

			/* Acceleration */
			vec3_scl(objects.mov[i], 6.0, vel);
			vec3_scl(vel, (TICK_TIME_S * t_speed), acl);

			/* Copy the acceleration to the player-object */
			vec3_add(v_old, acl, objects.vel[i]);


			vec3_cpy(pos, objects.pos[i]);
			vec3_scl(objects.vel[i], delt, del);
			vec3_add(pos, del, objects.pos[i]);

			x = objects.pos[i][0];
			z = objects.pos[i][2];
			objects.pos[i][1] = wld_get_height(x, z) + 2.2;

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
