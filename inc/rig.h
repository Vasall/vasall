#ifndef _RIG_H
#define _RIG_H

#include "vector.h"
#include "matrix.h"
#include <stdint.h>


#define JOINT_MAX_NUM 100

/*
 * A simple rig-struct containing the necessary data for the
 * animation of an object.
 *
 * @model: The slot of the model in the model-array
 * @anim: The index of the current animation
 * @prog: The current progress in rendering the animation
 * @ts: The timestamp of the last update of the animation
 * @jnt_num: The number of joints
 * @jnt_buf: The matrices for each joint
 */
struct model_rig {
	short     model;
	
	short     anim;
	float     prog;
	short     keyfr;
	uint32_t  ts;

	int       jnt_num;
	
	vec3_t    loc_pos[JOINT_MAX_NUM];
	vec4_t    loc_rot[JOINT_MAX_NUM];

	mat4_t    base_mat[JOINT_MAX_NUM];
	mat4_t    tran_mat[JOINT_MAX_NUM];
};


/*
 * Derive a rig from a model and setup the joint-matrices.
 *
 * @slot: The slot in the model-list to derive the rig from
 *
 * Returns: Either a pointer to an animation-runner or NULL if an error occurred 
 */
extern struct model_rig *rig_derive(short slot);


/*
 * Destroy a rig and free the allocated memory.
 *
 * @anim: Pointer to the rig
 */
extern void rig_free(struct model_rig *rig);


/*
 * Update the rig according to the current animation.
 *
 * @rig: Pointer to the rig to update
 */
extern void rig_update(struct model_rig *rig);

#endif
