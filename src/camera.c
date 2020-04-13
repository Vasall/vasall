#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Redefine external variable */
struct camera *camera;

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
int camCreate(float aov, float asp, float near, float far)
{

	/* Initialize the camera-struct */
	camera = calloc(1, sizeof(struct camera));
	if(camera == NULL) return(-1);

	/* Set the default position of the camera */
	vec3_set(camera->pos, 0.0, 0.0, 0.0);

	/* Set the direction-vector for the camera */	
	vec3_set(camera->forward, 1.0, 0.0, 0.0);
	vec3_set(camera->right, 0.0, 0.0, 1.0);

	/* Set the sensitivity of the mouse */
	camera->sens = 0.5;

	/* Calculate the initial distance */
	camera->dist = vec3_mag(camera->pos);

	/* Create the projection matrix */
	mat4_idt(camera->proj);
	camSetProjMat(aov, asp, near, far);

	/* Create the view-matrix */
mat4_idt(camera->view);
	camUpdViewMat();

	camera->trg_obj = NULL;

	return(0);
}

/*
 * Destroy the global camera-instance
 * and free the allocated memory.
 */
void camDestroy(void)
{
	free(camera);
}

/*
 * Get the projection matrix of the camera
 * and write it to the specified matrix.
 *
 * @mat: The matrix to write the result to
 */
void camGetProj(mat4_t mat)
{
	mat4_cpy(mat, camera->proj);
}

/*
 * Get the view matrix of the camera and
 * write it to the specified matrix.
 *
 * @mat: The matrix to write to
 */
void camGetView(mat4_t mat)
{
	mat4_cpy(mat, camera->view);
}

/* 
 * Get the position of the camera.
 *
 * @pos: The vector to write the position to
 */
void camGetPos(vec3_t pos)
{
	vec3_cpy(pos, camera->pos);
}

/*
 * Set the position of the camera.
 *
 * @pos: The vector containing the new position
 */
void camSetPos(vec3_t pos)
{
	vec3_cpy(camera->pos, pos);
}

/* 
 * Get the normalized direction the camera is looking.
 *
 * @dir: The vector to write the direction
 */
void camGetDir(vec3_t dir)
{
	vec3_cpy(dir, camera->forward);
}

/* 
 * Set the direction the camera is looking.
 *
 * @dir: The vector to read the direction from
 */
void camSetDir(vec3_t dir)
{
	vec3_t up = {0.0, 1.0, 0.0};

	vec3_cpy(camera->forward, dir);
	vec3_nrm(camera->forward, camera->forward);
	
	vec3_cross(up, camera->forward, camera->right);

	camUpdViewMat();
}

/*
 * Zoom the camera in or out and update the view matrix. 
 * This may change the camera's position
 *
 * @val: The zoom-value
 */
void camZoom(int val)
{
	camera->dist += val;
	if(camera->dist < 0.5) {
		camera->dist = 0.5;
	}

	camUpdate();
}

/*
 * Rotate the camera according to a yaw and pitch delta
 * and update the view matrix
 *
 * @d_yaw: the yaw delta in radians
 * @d_pitch: the pitch delta in radians
 */
void camRot(float d_yaw, float d_pitch)
{
	vec3_t stdup = {0.0, 1.0, 0.0};
	
	d_yaw *= camera->sens;
	d_pitch *= camera->sens;

	if(d_yaw != 0.0) {
		vec3_rot_y(camera->forward, d_yaw, camera->forward);
	}
	
	if(d_pitch != 0.0) {
		vec3_rot_axes(camera->forward, d_pitch, 
				camera->right, camera->forward);
	}

	vec3_nrm(camera->forward, camera->forward);

	vec3_cross(stdup, camera->forward, camera->right);
	vec3_nrm(camera->right, camera->right);

	camUpdate();
}

/*
 * Move the camera in a certain direction
 * and update the view matrix
 *
 * @dir: The direction to move the camera in.
 * 	(FORWARD, BACK, LEFT, RIGHT)
 */
void camMovDir(Direction dir)
{
	vec3_t movVec, up, forw, right;

	/* Vector to add to our position */
	vec3_set(movVec, 0.0, 0.0, 0.0);

	vec3_set(up, 0.0, 1.0, 0.0);
	vec3_set(forw, camera->forward[0], 0.0, camera->forward[2]);
	vec3_nrm(forw, forw);

	vec3_cross(up, forw, right);

	/* UP and DOWN not yet implemented */

	switch(dir) {
		case FORWARD:
			vec3_scl(forw, -1.0, movVec);
			break;
		case BACK:
			vec3_cpy(movVec, forw);
			break;
		case RIGHT:
			vec3_cpy(movVec, right);
			break;
		case LEFT:
			vec3_scl(right, -1.0, movVec);
			break;
		default:
			break;
	}

	camMov(movVec);
}

/*
 * Moves the camera according to vector
 *
 * @mov: The movement vector
 */
void camMov(vec3_t mov)
{
	if(camera->trg_obj != NULL) {
		printf("Trying to move camera freely with target entity set.\n");
		return;
	}

	vec3_add(camera->pos, mov, camera->pos);

	camUpdViewMat();
}

/*
 * Adjusts the camera's direction, so it looks
 * at the target point
 *
 * @trg: The target position to look at
 */
void camLookAt(vec3_t trg)
{
	vec3_t up = {0.0, 1.0, 0.0};

	if(camera->pos[0] == trg[0] && camera->pos[1] == trg[1] &&
			camera->pos[2] == trg[2]) {
		printf("Target equal to cam position!\n");
		vec3_set(camera->forward, 1.0, 0.0, 0.0);

		vec3_cross(up, camera->forward, camera->right);
		vec3_nrm(camera->right, camera->right);

		return;
	}

	vec3_sub(camera->pos, trg, camera->forward);
	vec3_nrm(camera->forward, camera->forward);

	vec3_cross(up, camera->forward, camera->right);
	vec3_nrm(camera->right, camera->right);
}

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
void camSetProjMat(float aov, float asp, float near, float far)
{
	float bottom, top, left, right, tangent;

	camera->aov = aov;
	camera->asp = asp;
	camera->near = near;
	camera->far = far;

	tangent = near * tan(aov * 0.5 * M_PI / 180);

	top = tangent;
	bottom = -top;
	right = top * asp;
	left = -right; 

	camera->proj[0x0] = (2 * near) / (right - left);
	camera->proj[0x5] = (2 * near) / (top - bottom); 	
	camera->proj[0x8] = (right + left) / (right - left); 
	camera->proj[0x9] = (top + bottom) / (top - bottom); 
	camera->proj[0xa] = -(far + near) / (far - near); 
	camera->proj[0xb] = -1; 
	camera->proj[0xe] = (-2 * far * near) / (far - near); 
	camera->proj[0xf] = 0;
}

/*
 * Calculate a view-matrix and write the
 * result into the specified camera
 */
void camUpdViewMat(void)
{
	vec3_t forw, right, up, stdup;

	/* The default up-vector */
	vec3_set(stdup, 0.0, 1.0, 0.0);

	vec3_cpy(forw, camera->forward);
	vec3_cpy(right, camera->right);

	vec3_cross(forw, right, up);

	camera->view[0x0] = right[0];
	camera->view[0x4] = right[1];
	camera->view[0x8] = right[2];

	camera->view[0x1] = up[0];
	camera->view[0x5] = up[1];
	camera->view[0x9] = up[2];

	camera->view[0x2] = forw[0];
	camera->view[0x6] = forw[1];
	camera->view[0xa] = forw[2];

	camera->view[0xc] = (-right[0] * camera->pos[0]) - (right[1] * camera->pos[1]) - (right[2] * camera->pos[2]);
	camera->view[0xd] = (-up[0] * camera->pos[0]) - (up[1] * camera->pos[1]) - (up[2] * camera->pos[2]);
	camera->view[0xe] = (-forw[0] * camera->pos[0]) - (forw[1] * camera->pos[1]) - (forw[2] * camera->pos[2]);
}

/* 
 * Set the camera's position and it's direction, so
 * it looks at the specified target
 *
 * @pos: The new position of the camera
 * @trg: The new target to focus on
*/
void camSet(vec3_t pos, vec3_t trg)
{	
	vec3_t up = {0.0, 1.0, 0.0};

	vec3_cpy(camera->pos, pos);
	
	vec3_sub(pos, trg, camera->forward);
	vec3_nrm(camera->forward, camera->forward);

	vec3_cross(up, camera->forward, camera->right);
	vec3_nrm(camera->right, camera->right);

	camUpdViewMat();
}

/*
 * Set an object as the target-point.
 *
 * @obj: Pointer to the object, to focus on
 */
void camTargetObj(struct object *obj)
{	
	camera->trg_obj = obj;
}

/*
 * Updates the camera according to it's set attributes.
 * This function may change the camera's position when a
 * target entity is set.
 */
void camUpdate(void)
{
	vec3_t tmp;

	if(camera->trg_obj != NULL) {
		vec3_scl(camera->forward, camera->dist, tmp);
		vec3_add(camera->trg_obj->pos, tmp, camera->pos);
		/* TODO do not target postion of object, but
		 * specific target point relative to it */
	} else {
		
	}
	camUpdViewMat();
}
