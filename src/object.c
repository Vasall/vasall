#include "object.h"
#include "world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Redefine global variables */
struct ht_t *objects = NULL;


struct object_table *obj_init(void)
{
	struct object_table *tbl;
	int i;

	if(!(tbl = malloc(sizeof(struct object_table))))
		return NULL;

	for(i = 0; i < OBJ_SLOTS; i++)
		tbl->mask[i] = OBJ_M_NONE;

	return tbl;
}

void obj_close(struct object_table *tbl)
{
	if(!tbl)
		return;

	free(tbl);
}

static short obj_get_slot(struct object_table *tbl)
{
	short i;

	for(i = 0; i < OBJ_SLOTS; i++) {
		if(tbl->mask[i] == OBJ_M_NONE)
			return i;
	}

	return -1;
}

short obj_set(struct object_table *tbl, uint32_t mask, vec3_t pos, short model,
		char *data, int len)
{
	short slot;

	if(!tbl)
		return -1;

	if((slot = obj_get_slot(tbl)) < 0)
		return -1;

	tbl->mask[slot] = mask;
	vec3_cpy(tbl->pos[slot], pos);
	vec3_set(tbl->vel[slot], 0, 0, 0);
	vec3_set(tbl->dir[slot], 1, 0, 1);
	tbl->model[slot] = model;
	tbl->anim[slot] = 0;
	tbl->prog[slot] = 0.0;
	mat4_idt(tbl->mat[slot]);

	tbl->len[slot] = 0;

	if(data && len) {
		len = (len > OBJ_DATA_MAX) ? (OBJ_DATA_MAX) : (len);
		tbl->len[slot] = len;
		memcpy(tbl->buf[slot], data, len);
	}

	return slot;
}

void obj_del(struct object_table *tbl, short slot)
{
	if(!tbl)
		return;

	tbl->mask[slot] = OBJ_M_NONE;
}

/* TODO: Limit Mask so it can't be set to 0 or negative values */
int obj_mod(struct object_table *tbl, short slot, short attr, 
		void *data, int len)
{
	if(!tbl)
		return -1;

	if(!tbl->mask[slot])
		return -1;

	switch(attr) {
		case(OBJ_ATTR_MASK):
			tbl->mask[slot] = *(uint32_t *)data;
			break;

		case(OBJ_ATTR_POS):
			vec3_cpy(tbl->pos[slot], data);
			break;
		
		case(OBJ_ATTR_VEL):
			vec3_cpy(tbl->vel[slot], data);
			break;
		
		case(OBJ_ATTR_DIR):
			vec3_cpy(tbl->dir[slot], data);
			break;
		
		case(OBJ_ATTR_BUF):
			tbl->len[slot] = len;
			memcpy(tbl->buf[slot], data, len);
			break;

		default:
			return -1;
	}

	return 0;
}

void obj_update_matrix(struct object_table *tbl, short slot)
{
	float rot;

	mat4_idt(tbl->mat[slot]);

	rot = atan2(-tbl->dir[slot][2], tbl->dir[slot][0]);
	tbl->mat[slot][0x0] =  cos(rot);
	tbl->mat[slot][0x2] = -sin(rot);
	tbl->mat[slot][0x8] =  sin(rot);
	tbl->mat[slot][0xa] =  cos(rot);

	tbl->mat[slot][0xc] = tbl->pos[slot][0];
	tbl->mat[slot][0xd] = tbl->pos[slot][1];
	tbl->mat[slot][0xe] = tbl->pos[slot][2];
}

void obj_print(struct object_table *tbl, short slot)
{
	printf("Display object %d:\n", slot);
	printf("Pos: "); vec3_print(tbl->pos[slot]); printf("\n");
	printf("Vel: "); vec3_print(tbl->vel[slot]); printf("\n");
	printf("Dir: "); vec3_print(tbl->dir[slot]); printf("\n");
}


void obj_sys_update(struct object_table *tbl, float delt)
{
	vec3_t pos;
	vec3_t del;
	int i;

	if(!tbl)
		return;
		
	for(i = 0; i < OBJ_SLOTS; i++) {
		if((tbl->mask[i] & OBJ_M_MOVE) == OBJ_M_MOVE) {
			float x, z;

			vec3_cpy(pos, tbl->pos[i]);
			vec3_scl(tbl->vel[i], delt, del);
			vec3_add(pos, del, tbl->pos[i]);

			x = tbl->pos[i][0];
			z = tbl->pos[i][2];
			tbl->pos[i][1] = wld_get_height(x, z) + 2.2;
			
			if(vec3_mag(del) > 0.0) {
				vec3_nrm(del, tbl->dir[i]);
				obj_update_matrix(tbl, i);
			}

		}
	}
}

void obj_sys_render(struct object_table *tbl)
{
	int i;

	if(!tbl)
		return;

	for(i = 0; i < OBJ_SLOTS; i++) {
		if((tbl->mask[i] & OBJ_M_MODEL) == OBJ_M_MODEL) {
			mdl_render(tbl->model[i], tbl->mat[i]);
		}
	}
}
