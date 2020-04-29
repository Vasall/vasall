#ifndef _V_CAMERA_H_
#define _V_CAMERA_H_

#include "defines.h"
#include "camera.h"
#include "vec.h"
#include "mat.h"
#include "object.h"

typedef enum {LEFT, RIGHT, FORWARD, BACK} Direction;

struct camera {
	/*
	 * The current position of the
	 * camera in the world.
	*/
	vec3_t pos;

	/*
	 * The direction vectors for the camera,
	 * where forwards points in the direction,
	 * the camera is looking at, while right is
	 * left-vector to the forward-vector.
	 */
	vec3_t forward;
	vec3_t right;

	/*
	 * The distance between the target and
	 * the camera.
	 */
	float dist;

	/*
	 * The sensivity of the camera, which
	 * influences how the camera can
	 * be controlled by the mouse.
	*/
	float sens;

	/*
	 * The angle of view used for the
	 * perspective matrix.
	 */
	float aov;

	/*
	 * The current aspect ratio.
	 */
	float asp;

	/*
	 * The near-limit used when
	 * checking for clipping.
	 */
	float near;

	/*
	 * The far-limit used when
	 * checking for clipping.
	 */
	float far;

	/*
	 * The projection matrix.
	 */
	mat4_t proj;

	/*
	 * The current viewmatrix.
	 */
	mat4_t view;

	/*
	 * The target entity. When this is not NULL, the camera will
	 * always follow this entity
	 */
	short trg_obj;
};


/* The global camera-struct */
V_GLOBAL struct camera *camera;

/*
 * Initialize the global camera-instance.
 *
 * @aov: The angle of view
 * @asp: The current aspect ratio
 * @near: The near-limit
 * @far: The far-limit
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
V_API int cam_init(float aov, float asp, float near, float far);

/*
 * Destroy the global camera-instance
 * and free the allocated memory.
 */
V_API void cam_close(void);

/*
 * Get the projection matrix of the camera
 * and write it to the specified matrix.
 *
 * @mat: The matrix to write the result to
 */
V_API void cam_get_proj(mat4_t mat);

/*
 * Get the view matrix of the camera and
 * write it to the specified matrix.
 *
 * @mat: The matrix to write to
 */
V_API void cam_get_view(mat4_t mat);

/*
 * Set the position of the camera.
 *
 * @pos: The vector containing the new position
 */
V_API void cam_set_pos(vec3_t pos);

/* 
 * Get the position of the camera.
 *
 * @pos: The vector to write the position to
 */
V_API void cam_get_pos(vec3_t pos);

/* 
 * Get the normalized direction the camera is looking.
 *
 * @dir: The vector to write the direction
 */
V_API void cam_set_dir(vec3_t dir);

/*
 * Get the direction, the camera is currently looking at.
 *
 * @dir: A vector to write the direction to
 */
V_API void cam_get_dir(vec3_t dir);

/* 
 * Set the direction the camera is looking.
 *
 * @dir: The vector to read the direction from
 */
V_API void cam_mouse_moved(int delx, int dely);

/*
 * Zoom the camera in or out and update the view matrix. 
 * This may change the camera's position
 *
 * @val: The zoom-value
 */
V_API void cam_zoom(int val);

/*
 * Rotate the camera according to a yaw and pitch delta
 * and update the view matrix
 *
 * @d_yaw: the yaw delta in radians
 * @d_pitch: the pitch delta in radians
 */
V_API void cam_rot(float d_yaw, float d_pitch);

/*
 * Move the camera in a certain direction
 * and update the view matrix
 *
 * @dir: The direction to move the camera in.
 * 	(FORWARD, BACK, LEFT, RIGHT)
 */
V_API void cam_mov_dir(Direction dir);

/*
 * Moves the camera according to vector
 *
 * @mov: The movement vector
 */
V_API void cam_mov(vec3_t mov);

/*
 * Create a new projection-matrix and
 * write the result into the given
 * matrix.
 *
 * @aov: Angle of view in degree
 * @asp: The aspect ratio of the window
 * @near: The near-limit
 * @far: The far-limit
 */
V_API void cam_proj_mat(float aov, float asp, float near, float far);

/*
 * Calculate a view-matrix and write the
 * result into the specified camera
 */
V_API void cam_update_view(void);

/*
 * Focus the camera onto a certain position.
 *
 * @trg: The vector containign to position
 */
V_API void cam_look_at(vec3_t trg);

/* 
 * Set the camera's position and it's direction, so it looks at the specified
 * target.
 *
 * @pos: The new position of the camera
 * @trg: The new target to focus on
*/
V_API void cam_set(vec3_t pos, vec3_t trg);

/*
 * Set an object as the target-point.
 *
 * @obj: The slot of the object in the object-table
 */
V_API void cam_trg_obj(short obj);

/*
 * Updates the camera according to it's set attributes.
 * This function may change the camera's position when a
 * target entity is set.
 */
V_API void cam_update(void);

#endif
