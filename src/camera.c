#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 * Create a new camera and initialize the
 * members of the struct with default values.
 *
 * @aov: The angle of view
 * @asp: The current aspect ratio
 * @near: The near-limit
 * @far: The far-limit
 *
 * Returns: Either the pointer to the created
 * 	camera-struct or null
 */
Camera *camCreate(float aov, float asp, float near, float far)
{
	Camera *cam;

	/* Initialize the camera-struct */
	cam = calloc(1, sizeof(Camera));
	if(cam == NULL) return(NULL);

	/* Set the default position of the camera */
	vecSet(cam->pos, 0.0, 0.0, 0.0);

	/* Set the direction-vector */
	vecSet(cam->dir, 1.0, 0.0, 0.0);

	/* Set the sensitivity of the mouse */
	cam->sens = 0.5;

	cam->dist = vecMag(cam->pos);

	/* Create the projection matrix */
	mat4Idt(cam->proj);
	camSetProjMat(cam, aov, asp, near, far);

	/* Create the view-matrix */
	mat4Idt(cam->view);
	camUpdViewMat(cam);

	cam->trg_obj = NULL;

	return(cam);
}

/*
 * Destroy a camera and free
 * the allocated memory.
 *
 * @cam: Pointer to the camera 
 * 	to destroy
 */
void camDestroy(Camera *cam)
{
	free(cam);
}

/*
 * Get the projection matrix of the camera
 * and write it to the specified matrix.
 *
 * @cam: Pointer to the camera
 * @mat: The matrix to write the result to
 */
void camGetProj(Camera *cam, Mat4 mat)
{
	mat4Cpy(mat, cam->proj);
}

/*
 * Get the view matrix of a camera and
 * write it to the specified matrix.
 *
 * @cam: Pointer to the camera
 * @mat: The matrix to write to
 */
void camGetView(Camera *cam, Mat4 mat)
{
	mat4Cpy(mat, cam->view);
}

/* 
 * Get the position of the camera.
 *
 * @cam: Pointer to the camera
 * @pos: The vector to write the position to
 */
void camGetPos(Camera *cam, Vec3 pos)
{
	vecCpy(pos, cam->pos);
}

/*
 * Set the position of the camera.
 *
 * @cam: Pointer to the camera
 * @pos: The vector containing the new position
 */
void camSetPos(Camera *cam, Vec3 pos)
{
	vecCpy(cam->pos, pos);
}

/* 
 * Get the normalized direction the camera is looking.
 *
 * @cam: Pointer to the camera
 * @dir: The vector to write the direction
 */
void camGetDir(Camera *cam, Vec3 dir)
{
	vecCpy(dir, cam->dir);
	vecNrm(dir, dir);
}

/*
 * Zoom the camera in or out and
 * update the view matrix.
 * This may change the camera's position
 *
 * @cam: Pointer to the camera to modify
 * @val: The zoom-value
 */
void camZoom(Camera *cam, int val)
{
	Vec3 tmp;

	cam->dist += val;
	if(cam->dist < 0.5)
		cam->dist = 0.5;

	camUpdate(cam);
	return;

	if(cam->trg_obj != NULL) {
		cam->dist += val;
		if(cam->dist < 0.5)
			cam->dist = 0.5;

		camLookAt(cam, cam->trg_obj->pos);
		vecNrm(cam->dir, cam->dir);
		vecScl(cam->dir, cam->dist, tmp);
		vecAdd(cam->trg_obj->pos, tmp, cam->pos);
	} else {
		/* FIXME this is not a zoom, but movement
		 * Zooming shouldn't change the camera's position
		 * when moving freely, but instead change the
		 * projection matrix to have a smaller fov */
		vecNrm(cam->dir, tmp);
		vecScl(tmp, val, tmp);
		vecAdd(cam->pos, tmp, cam->pos);
	}

	camUpdViewMat(cam);
}

/*
 * Rotate the camera according to a yaw and pitch delta
 * and update the view matrix
 *
 * @cam: The camera to rotate
 * @d_yaw: the yaw delta in radians
 * @d_pitch: the pitch delta in radians
 */
void camRot(Camera *cam, float d_yaw, float d_pitch)
{
	Vec3 left, stdup;
	vecSet(stdup, 0.0, 1.0, 0.0);
	
	d_yaw *= cam->sens;
	d_pitch *= cam->sens;

	if(d_yaw != 0.0)
		vecRotY(cam->dir, d_yaw, cam->dir);
	
	if(d_pitch != 0.0) {
		vecCross(stdup, cam->dir, left);
		vecNrm(left, left);
		vecRotAxis(cam->dir, d_pitch, left, cam->dir);
	}
	vecNrm(cam->dir, cam->dir);

	camUpdate(cam);
}

/*
 * Move the camera in a certain direction
 * and update the view matrix
 *
 * @cam: The camera to move
 * @dir: The direction to move the camera in.
 * 	(FORWARD, BACK, LEFT, RIGHT)
 */
void camMovDir(Camera *cam, Direction dir)
{
	Vec3 movVec, up, forw, left;

	/* Vector to add to our position */
	vecSet(movVec, 0.0, 0.0, 0.0);

	vecSet(up, 0.0, 1.0, 0.0);
	vecSet(forw, cam->dir[0], 0.0, cam->dir[2]);
	vecNrm(forw, forw);
	vecCross(up, forw, left);

	/* UP and DOWN not yet implemented */

	switch(dir) {
		case FORWARD:
			vecScl(forw, -1.0, movVec);
			break;
		case BACK:
			vecCpy(movVec, forw);
			break;
		case RIGHT:
			vecCpy(movVec, left);
			break;
		case LEFT:
			vecScl(left, -1.0, movVec);
			break;
		default:
			break;
	}

	camMov(cam, movVec);
}

/*
 * Moves the camera according to vector
 *
 * @cam: The camera to move
 * @mov: The movement vector
 */
void camMov(Camera *cam, Vec3 mov)
{
	if(cam->trg_obj != NULL) {
		printf("Trying to move camera freely with target entity set.\n");
		return;
	}

	vecAdd(cam->pos, mov, cam->pos);

	camUpdViewMat(cam);
}

/*
 * Adjusts the camera's direction, so it looks
 * at the target point
 *
 * @cam: The camera
 * @trg: The target position to look at
 */
void camLookAt(Camera *cam, Vec3 trg)
{
	if(cam->pos[0] == trg[0] &&
			cam->pos[1] == trg[1] &&
			cam->pos[2] == trg[2]) {
		printf("Target equal to cam position!\n");
		vecSet(cam->dir, 1.0, 0.0, 0.0);
		return;
	}
	vecSub(cam->pos, trg, cam->dir);
}

/*
 * Create a new projection-matrix and
 * write the result into the given
 * matrix.
 *
 * @cam: The camera to modify
 * @aov: Angle of view in degree
 * @asp: The aspect ratio of the window
 * @near: The near-limit
 * @far: The far-limit
 */
void camSetProjMat(Camera *cam, float aov, float asp, float near, 
		float far)
{
	float bottom, top, left, right, tangent;

	cam->aov = aov;
	cam->asp = asp;
	cam->near = near;
	cam->far = far;

	tangent = near * tan(aov * 0.5 * M_PI / 180);

	top = tangent;
	bottom = -top;
	right = top * asp;
	left = -right; 

	cam->proj[0x0] = (2 * near) / (right - left);
	cam->proj[0x5] = (2 * near) / (top - bottom); 	
	cam->proj[0x8] = (right + left) / (right - left); 
	cam->proj[0x9] = (top + bottom) / (top - bottom); 
	cam->proj[0xa] = -(far + near) / (far - near); 
	cam->proj[0xb] = -1; 
	cam->proj[0xe] = (-2 * far * near) / (far - near); 
	cam->proj[0xf] = 0;

}

/*
 * Calculate a view-matrix and write the
 * result into the specified camera
 *
 * @cam: The camera to update the view matrix of
 */
void camUpdViewMat(Camera *cam)
{
	Vec3 forw, left, up, stdup;

	/* The default up-vector */
	vecSet(stdup, 0.0, 1.0, 0.0);

	vecNrm(cam->dir, forw);
	vecCross(stdup, forw, left);
	vecNrm(left, left);

	vecCross(forw, left, up);

	cam->view[0x0] = left[0];
	cam->view[0x4] = left[1];
	cam->view[0x8] = left[2];

	cam->view[0x1] = up[0];
	cam->view[0x5] = up[1];
	cam->view[0x9] = up[2];

	cam->view[0x2] = forw[0];
	cam->view[0x6] = forw[1];
	cam->view[0xa] = forw[2];

	cam->view[0xc]= -left[0] * cam->pos[0] - left[1] * cam->pos[1] - left[2] * cam->pos[2];
	cam->view[0xd]= -up[0] * cam->pos[0] - up[1] * cam->pos[1] - up[2] * cam->pos[2];
	cam->view[0xe]= -forw[0] * cam->pos[0] - forw[1] * cam->pos[1] - forw[2] * cam->pos[2];
}

/* 
 * Set the camera's position and it's direction, so
 * it looks at the specified target
 *
 * @cam: Pointer to the camera
 * @pos: The new position of the camera
 * @trg: The new target to focus on
*/
void camSet(Camera *cam, Vec3 pos, Vec3 trg)
{	
	vecCpy(cam->pos, pos);
	
	vecSub(pos, trg, cam->dir);
	vecNrm(cam->dir, cam->dir);

	camUpdViewMat(cam);
}

/*
 * Updates the camera according to it's set attributes.
 * This function may change the camera's position when a
 * target entity is set.
 *
 * @cam: The camera to update
 */
void camUpdate(Camera *cam)
{
	Vec3 tmp;

	if(cam->trg_obj != NULL) {
		vecNrm(cam->dir, cam->dir);
		vecScl(cam->dir, cam->dist, tmp);
		vecAdd(cam->trg_obj->pos, tmp, cam->pos);
		/* TODO do not target postion of object, but
		 * specific target point relative to it */
	} else {
		
	}
	camUpdViewMat(cam);
}
