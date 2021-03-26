#include "object_utils.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * object-movement-log
 */

extern void obj_log_reset(short slot)
{
	objects.log[slot].start = 0;
	objects.log[slot].num = 0;
}

extern short obj_log_set(short slot, uint32_t ts, vec3_t pos, vec3_t vel,
		vec2_t mov, vec3_t dir)
{
	short i;
	short islot;
	short tmp;

	/*
	 * Check if an entry with the timestamp is already in the list. If that
	 * is the case, update the value of the entry;
	 */	
	for(i = 0; i < OBJ_LOG_SLOTS; i++) {
		if(objects.log[slot].ts[i] == ts) {
			/* Update values */
			vec3_cpy(objects.log[slot].pos[i], pos);
			vec3_cpy(objects.log[slot].vel[i], vel);

			vec2_cpy(objects.log[slot].mov[i], mov);
			vec3_cpy(objects.log[slot].dir[i], dir);
			return i;
		}
	}

	/*
	 * If the timestamp is smaller than the last entry in the list,
	 * just return.
	 */
	tmp = (objects.log[slot].start + objects.log[slot].num - 1) %
		OBJ_LOG_SLOTS;
	if(objects.log[slot].ts[tmp] > ts) {
		return -1;
	}

	/* Get the slot at the end of the list */
	islot = (objects.log[slot].start + objects.log[slot].num) %
		OBJ_LOG_SLOTS;

	/* Copy data to the slot */
	objects.log[slot].ts[islot] = ts;

	vec3_cpy(objects.log[slot].pos[islot], pos);
	vec3_cpy(objects.log[slot].vel[islot], vel);

	vec2_cpy(objects.log[slot].mov[islot], mov);
	vec3_cpy(objects.log[slot].dir[islot], dir);

	/* 
	 * Move start of list to next slot so first one can be overwritten.
	 */
	if(objects.log[slot].num + 1 >= OBJ_LOG_SLOTS) {
		objects.log[slot].start = (objects.log[slot].start + 1) %
			OBJ_LOG_SLOTS;
	}
	else {
		objects.log[slot].num += 1;
	}

	return islot;
}

/*
 * TODO: What if the timestamp is older than the oldest log-entry?!
 */
extern char obj_log_near(short slot, uint32_t ts)
{
	int i;
	char near = objects.log[slot].start;

	for(i = 0; i < objects.log[slot].num; i++) {
		char tmp = (objects.log[slot].start + i) % OBJ_LOG_SLOTS;

		if(objects.log[slot].ts[tmp] > ts)
			break;

		near = tmp;
	}

	return near;
}

extern void obj_log_col(uint32_t ts, char *logi)
{
	int i;

	/* Collect log-entries closest to the timestamp */
	for(i = 0; i < OBJ_SLOTS; i++) {
		logi[i] = -1;

		if(objects.mask[i] != 0) {
			logi[i] = obj_log_near(i, ts);
		}
	}
}

extern void obj_log_cpy(short slot, short i, uint32_t *ts, vec3_t pos, vec3_t vel,
		vec2_t mov, vec3_t dir)
{
	struct object_log *log = &objects.log[slot];

	*ts = log->ts[i];

	vec3_cpy(pos, log->pos[i]);
	vec3_cpy(vel, log->vel[i]);

	vec2_cpy(mov, log->mov[i]);
	vec3_cpy(dir, log->dir[i]);
}
