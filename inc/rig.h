#ifndef _MDLRIG_H
#define _MDLRIG_H

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
	short model;
	
	short anim;
	float prog;
	char c;
	uint32_t ts;

	int jnt_num;
	vec3_t *jnt_pos;
	vec4_t *jnt_rot;
	mat4_t jnt_mat[JOINT_MAX_NUM];
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
 * 
 */
extern void rig_update(struct model_rig *rig);

#endif
