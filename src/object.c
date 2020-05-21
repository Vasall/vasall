#include "object.h"
#include "world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct object_table objects;


extern int obj_init(void)
{
	int i;

	for(i = 0; i < OBJ_SLOTS; i++)
		objects.mask[i] = OBJ_M_NONE;

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

	if((slot = obj_get_slot()) < 0)
		return -1;

	objects.mask[slot] = mask;
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

	x = objects.pos[slot][0];
	z = objects.pos[slot][2];
	objects.pos[slot][1] = wld_get_height(x, z) + 2.2;
	obj_update_matrix(slot);
	return slot;
}


extern void obj_del(short slot)
{
	if(obj_check_slot(slot))
		return;

	objects.mask[slot] = OBJ_M_NONE;
}


extern int obj_mod(short slot, short attr, void *data, int len)
{
	if(obj_check_slot(slot))
		return -1;

	if(!objects.mask[slot])
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
		
		case(OBJ_ATTR_DIR):
			vec3_cpy(objects.dir[slot], data);
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


extern void obj_update_matrix(short slot)
{
	float rot;

	if(obj_check_slot(slot))
		return;

	mat4_idt(objects.mat[slot]);

	rot = atan2(-objects.dir[slot][2], objects.dir[slot][0]);
	objects.mat[slot][0x0] =  cos(rot);
	objects.mat[slot][0x2] = -sin(rot);
	objects.mat[slot][0x8] =  sin(rot);
	objects.mat[slot][0xa] =  cos(rot);

	objects.mat[slot][0xc] = objects.pos[slot][0];
	objects.mat[slot][0xd] = objects.pos[slot][1];
	objects.mat[slot][0xe] = objects.pos[slot][2];
}


extern void obj_print(short slot)
{
	if(obj_check_slot(slot))
		return;

	printf("Display object %d:\n", slot);
	printf("Pos: "); vec3_print(objects.pos[slot]); printf("\n");
	printf("Vel: "); vec3_print(objects.vel[slot]); printf("\n");
	printf("Dir: "); vec3_print(objects.dir[slot]); printf("\n");
}


extern void obj_sys_update(float delt)
{
	vec3_t pos;
	vec3_t del;
	int i;
		
	for(i = 0; i < OBJ_SLOTS; i++) {
		if((objects.mask[i] & OBJ_M_MOVE) == OBJ_M_MOVE) {
			float x, z;

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


extern void obj_sys_render(void)
{
	int i;

	for(i = 0; i < OBJ_SLOTS; i++) {
		if((objects.mask[i] & OBJ_M_MODEL) == OBJ_M_MODEL) {
			mdl_render(objects.model[i], objects.mat[i]);
		}
	}
}
