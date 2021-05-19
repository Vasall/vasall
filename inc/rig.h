#ifndef _RIG_H
#define _RIG_H

#include "vector.h"
#include "matrix.h"
#include <stdint.h>


#define JOINT_MAX_NUM 100

/*
 * A simple rig-struct containing the necessary data for the
 * animation of an object.
 */
struct model_rig {
	short     model;
	
	short     anim;
	float     prog;
	short     keyfr;
	uint32_t  ts;

	char      jnt_m[JOINT_MAX_NUM];
	int       jnt_num;
	vec3_t    loc_pos[JOINT_MAX_NUM];
	vec4_t    loc_rot[JOINT_MAX_NUM];

	mat4_t    base_mat[JOINT_MAX_NUM];
	mat4_t    trans_mat[JOINT_MAX_NUM];

	/*
	 * Hooks
	 */
	short     hook_num;
	vec3_t    *hook_pos;
	vec3_t    *hook_dir;
	mat4_t    *hook_base_mat;
	mat4_t    *hook_loc_mat;
	mat4_t    *hook_trans_mat;
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
 * @viewp: The position to look at relative to the model
 */
extern void rig_update(struct model_rig *rig, vec3_t viewp);


/*
 * Multiply the current joint-matrices with the given transformation-matrix.
 *
 * @rig: Pointer to the rig, to modify
 * @m: The transformation-matrix
 */
extern void rig_mult_mat(struct model_rig *rig, mat4_t m);


/*
 * Update the rotation of the hook, so the forward-vector will point to the
 * specified position in model-space, and use inverse-kinematics to move the
 * joints accordingly.
 *
 * @rig: Pointer to a rig containing the hooks and joints
 * @hk: The index of the hook
 * @pos: The position to point the forward-vector to
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int rig_hk_lookat(struct model_rig *rig, short hk, vec3_t pos);


#endif
