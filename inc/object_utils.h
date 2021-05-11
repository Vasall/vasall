#ifndef _OBJECT_UTILS_H
#define _OBJECT_UTILS_H

#include "object.h"

/* 
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 *             
 *            OBJECT_MOVEMENT_LOG
 *
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 */

extern void obj_log_reset(short slot);

extern short obj_log_set(short slot, uint32_t ts, vec3_t pos, vec3_t vel,
		vec2_t mov, vec3_t dir);

extern char obj_log_near(short slot, uint32_t ts);

extern void obj_log_col(uint32_t ts, char *logi);

extern void obj_log_cpy(short slot, short i, uint32_t *ts, vec3_t pos, vec3_t vel,
		vec2_t mov, vec3_t dir);

/*
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 *             
 *              OBJECT_HANDHELD
 *
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- 
 */


/*
 * Calculate the point the object is currently aiming at both in world-space and
 * model-space and write it to the object.
 *
 * @slot: The object-slot
 */
extern void obj_calc_aim(short slot);

/*
 * Update the handheld-data based on the current animation.
 *
 * @slot: The object-slot
 */
extern void obj_hnd_update(short slot);


#endif /* _OBJECT_UTILS_H */
