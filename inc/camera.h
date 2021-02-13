#ifndef _CAMERA_H
#define _CAMERA_H

#include "camera.h"
#include "vector.h"
#include "matrix.h"
#include "object.h"

enum cam_mode {
	CAM_FPV,       /* first-person-view */
	CAM_TPV        /* third-person-view */
};

struct camera_wrapper {
	enum cam_mode mode;

	vec3_t pos;
	vec3_t dir;
	vec3_t forward;
	vec3_t right;

	float aov;
	float asp;
	float near;
	float far;

	float dist;
	short trg_obj;

	mat4_t proj;
	mat4_t view;
	
	float sens;
};


/* The global camera-wrapper-instance */
extern struct camera_wrapper camera;


/*
 * Initialize the global camera-wrapper.
 *
 * @aov: The angle-of-view
 * @asp: The aspect-ratio of the view
 * @near: The near-limit
 * @far: The far-limit
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int cam_init(float aov, float asp, float near, float far);


/*
 * Close the camera and reset the camera-instance.
 */
extern void cam_close(void);


/*
 * Get the current projection-matrix of the camera.
 *
 * @mat: A matrix to write the projection-matrix to
 */
extern void cam_get_proj(mat4_t mat);


/*
 * Get the current view-matrix of the camera.
 *
 * @mat: A matrix to write the view-matrix to
 */
extern void cam_get_view(mat4_t mat);


/*
 * Set the position of the camera. This function will then also update the
 * view-matrix.
 *
 * @pos: The new position of the camera
 */
extern void cam_set_pos(vec3_t pos);


/*
 * Get the current position of the camera.
 *
 * @pos: A vector to write the current position to
 */
extern void cam_get_pos(vec3_t pos);


/*
 * Set the direction of the camera. This function will then also update the
 * view-matrix.
 *
 * @dir: the new direction of the camera
 */
extern void cam_set_dir(vec3_t dir);


/*
 * Get the current direction of the camera.
 *
 * @dir: A vector to write the current direction to
 */
extern void cam_get_dir(vec3_t dir);


/*
 * Change the distance of the camera to the focused object. Note that this
 * function will only work, if the camera is currently targeting an object.
 * This function will then update the view-matrix of the camera.
 *
 * @val: The new distance to the target
 */
extern void cam_zoom(int val);


/*
 * Change the rotation of the camera. This function will then update the
 * view-matrix of the camera.
 *
 * @d_yaw: The delta-yaw value (up and down)
 * @d_pitch: The delta pitch value (left and right)
 */
extern void cam_rot(float d_yaw, float d_pitch);


/*
 * Move the camera with a certain vector. This function will then update the
 * view-matrix.
 *
 * @mov: The movement-vector the vector is going to be moved by
 */
extern void cam_mov(vec3_t mov);


/*
 * Focus the camera on a given position and recalculate the view-matrix.
 *
 * @trg: The position to focus on
 */
extern void cam_look_at(vec3_t trg);


/*
 * Change the projection matrix and recalculate the projection-matrix with new
 * values.
 *
 * @aov: The new angle-of-view
 * @asp: The new aspect-ation
 * @near: The new near-limit
 * @far: The new far-limit
 */
extern void cam_proj_mat(float aov, float asp, float near, float far);


/*
 * This function will recalculate the view-matrix of the camera.
 */
extern void cam_update_view(void);


/*
 * Set the camera-position and the position to focus the camera on.
 *
 * @pos: The new position of the camera
 * @trg: The position to focus the camera on
 */
extern void cam_set(vec3_t pos, vec3_t trg);


/*
 * Target an object and focus the camera onto this object. This function will
 * then lock the camera onto the object and update the camera if the position
 * of the object changes.
 */
extern void cam_trg_obj(short obj);


extern void cam_tgl_view(void);

/*
 * Update the camera.
 */
extern void cam_update(void);

#endif
