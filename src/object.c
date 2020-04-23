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
	int i;

	if(!tbl)
		return;
	
	for(i = 0; i < OBJ_SLOTS; i++) {
		if((tbl->mask[i] & OBJ_M_DATA) == OBJ_M_DATA)
			free(tbl->data[i]);
	}

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
	
	tbl->len[slot] = 0;
	tbl->data[slot] = NULL;

	if(data && len) {
		len = (len > OBJ_DATA_MAX) ? (OBJ_DATA_MAX) : (len);
		tbl->len[slot] = len;
		memcpy(tbl->[slot], data, len);
	}

	return slot;

err_clear_slot:
	tbl->mask[slot] = OBJ_M_NONE;
	return -1;
}

int obj_set(char *key, char* mdl, vec3_t pos)
{
	struct object *obj = NULL;

	obj = malloc(sizeof(struct object));
	if(obj == NULL)
		return -1;

	memset(obj, 0, sizeof(struct object));

	strcpy(obj->key, key);

	vec3_cpy(obj->pos, pos);
	vec3_set(obj->vel, 0.0, 0.0, 0.0);
	vec3_set(obj->dir, 1.0, 1.0, 1.0);
	vec3_set(obj->scl, 1.0, 1.0, 1.0);
	vec3_set(obj->rot, 0.0, 0.0, 0.0);

	if((obj->model = mdl_get(mdl)) == NULL)
		goto err_free_obj;

	obj_update_mat(obj);

	if(ht_set(objects, key, (uint8_t *)obj, sizeof(struct object)) < 0)
		goto err_free_obj;

	return 0;	

err_free_obj:
	free(obj);
	return -1;
}

void obj_del(char *key)
{
	if(key) {}
}

struct object *obj_get(char *key)
{
	struct object *obj;

	if(ht_get(objects, key, (uint8_t **)&obj, NULL) < 0)
		return NULL;

	return obj;
}

void obj_update(struct object *obj, float delt)
{
	vec3_t del;
	
	vec3_cpy(del, obj->vel);
	vec3_scl(del, delt, del);

	obj_add_pos(obj, del);

	obj->pos[1] = wld_get_height(obj->pos[0], obj->pos[2]) + 2.2;
	
	if(vec3_mag(del) > 0.0) {
		vec3_nrm(del, obj->dir);
		obj->rot[1] = atan2(-obj->dir[2], obj->dir[0]);
	}

	obj_update_mat(obj);
}

void obj_render(struct object *obj)
{
	mdl_render(obj->model, obj->matrix);
}

/* =============================================== */
/*                     POSITION                    */
/* =============================================== */

void obj_set_pos(struct object *obj, vec3_t pos)
{
	vec3_cpy(obj->pos, pos);
	obj_update_mat(obj);
}

void obj_get_pos(struct object *obj, vec3_t pos)
{
	vec3_cpy(pos, obj->pos);
}

void obj_add_pos(struct object *obj, vec3_t del)
{
	vec3_add(obj->pos, del, obj->pos);
	obj_update_mat(obj);
}

/* =============================================== */
/*                     ROTATION                    */
/* =============================================== */

void obj_set_rot(struct object *obj, vec3_t rot)
{
	vec3_cpy(obj->rot, rot);
	obj_update_mat(obj);
}

void obj_get_rot(struct object *obj, vec3_t rot)
{
	vec3_cpy(rot, obj->rot);
}

void obj_add_rot(struct object *obj, vec3_t del)
{
	vec3_add(obj->rot, del, obj->rot);
	obj_update_mat(obj);
}

/* =============================================== */
/*                     VELOCITY                    */
/* =============================================== */

void obj_set_vel(struct object *obj, vec3_t vel)
{
	vec3_cpy(obj->vel, vel);
}

void obj_get_vel(struct object *obj, vec3_t vel)
{
	vec3_cpy(vel, obj->vel);
}

void obj_add_vel(struct object *obj, vec3_t del)
{
	vec3_add(obj->vel, del, obj->vel);
}

/* =============================================== */
/*                     DIRECTION                   */
/* =============================================== */

void obj_set_dir(struct object *obj, vec3_t dir)
{
	vec3_cpy(obj->dir, dir);
	vec3_nrm(obj->dir, obj->dir);
}

void obj_get_dir(struct object *obj, vec3_t dir)
{
	vec3_cpy(dir, obj->dir);
}

/* =============================================== */
/*                       MODEL                     */
/* =============================================== */

void obj_set_model(struct object *obj, char *mdl)
{
	obj->model = mdl_get(mdl);
}

void obj_get_mat(struct object *obj, mat4_t mat)
{
	mat4_cpy(mat, obj->matrix);
}

void obj_update_mat(struct object *obj)
{
	vec3_t rot;

	vec3_cpy(rot, obj->rot);
	mat4_idt(obj->matrix);

	obj->matrix[0x0] =  cos(rot[1]);
	obj->matrix[0x2] = -sin(rot[1]);
	obj->matrix[0x8] =  sin(rot[1]);
	obj->matrix[0xa] =  cos(rot[1]);

	obj->matrix[0xc] = obj->pos[0];
	obj->matrix[0xd] = obj->pos[1];
	obj->matrix[0xe] = obj->pos[2];
}

void obj_print(struct object *obj)
{
	printf("Key: %s\n", obj->key);
	printf("Pos: "); vec3_print(obj->pos); printf("\n");
	printf("Rot: "); vec3_print(obj->rot); printf("\n");
	printf("Vel: "); vec3_print(obj->vel); printf("\n");
}
