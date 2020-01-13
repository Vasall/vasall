#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "camera.h"

/*
 * Create a new camera and initialize the
 * values of the struct with the default
 * settings and set the position of the
 * camera.
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
 	Vec3 del;

	/* Initialize the camera-struct */
	cam = calloc(1, sizeof(Camera));
	if(cam == NULL) return(NULL);

	/* Set the default position of the target */
	cam->trg[0] = 0;
	cam->trg[1] = 0;
	cam->trg[2] = 0;

	/* Set the default position of the camera */
	cam->pos[0] = 0.0;
	cam->pos[1] = 1.0;
	cam->pos[2] = 0.0;

	/* Get the connecting vector between the camera and the target */
	vecSub(cam->pos, cam->trg, del);

	/* The normalized direction vector from the target to the camera */
	vecNrm(del, cam->dir);

	/* Calculate the distance between the target and the camera */
	cam->dist = vecMag(del);

	/* Set the sensitivity of the mouse */
	cam->sensitivity = 0.2;	

	/* Create the projection matrix */
	mat4Idt(cam->proj);
	camSetProjMat(cam, aov, asp, near, far);

	/* Create the view-matrix */
	mat4Idt(cam->view);
	camSetViewMat(cam, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);

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
void camGetPos(Camera* cam, Vec3 pos)
{
	vecCpy(pos, cam->pos);
}

/* 
 * Get the direction the camera is looking.
 * This function will return the normalvector
 * pointing from the camera to the target.
 *
 * @cam: Pointer to the camera
 * @dir: The vector write the direction to
 */
void camGetDir(Camera *cam, Vec3 dir)
{
	vecCpy(dir, cam->dir);
	vecNrm(dir, dir);
}

/*
 * Adjust the rotation of the camera.
 * This function will adjust the rotation of
 * the camera to the movement of the mouse.
 *
 * @cam: Pointer to the camera to modify
 * @delx: The number of pixels, the mouse has
 * 	been moved on the x-axis
 * @dely: The number of pixels, the mouse has
 * 	been moved on the y-axis
 */
void camMouseMoved(Camera *cam, int delx, int dely)
{
	/* Rotate camera */
	cam->rot[0] += ((float)dely) * cam->sensitivity;
	cam->rot[1] += ((float)delx) * cam->sensitivity;

	/* Limit looking down to vertically down */
	if (cam->rot[0] > 90) {
		cam->rot[0] = 90;
	}
	if (cam->rot[0] < -90) {
		cam->rot[0] = -90;
	}

	/* Modulo of int is not available */
	if (cam->rot[1] < 0) {
		cam->rot[1] += 360;
	}
	if (cam->rot[1] > 360) {
		cam->rot[1] += -360;
	}
}

/*
 * Change the zoom of the camera,
 * when scrolling. This function
 * will then also reposition the 
 * camera and update the view-matrix.
 *
 * @cam: Pointer to the camera to modify
 * @val: The scrolling-value
 */
void camZoom(Camera *cam, int val)
{
	Vec3 tmp;

	vecScl(cam->dir, val, tmp);
	camMov(cam, tmp, 0);
	cam->dist += vecMag(cam->dir) * val;

	camUpdPos(cam);
}


void camMovDir(Camera *cam, Direction dir, int mov_trg) 
{
	Vec3 movVec, up, forw, left;

	/* Vector to add to our position */
	vecSet(movVec, 0.0, 0.0, 0.0);

	vecSet(up, 0.0, 1.0, 0.0);
	vecSet(forw, cam->dir[0], 0.0, cam->dir[2]);
	vecCross(up, forw, left);
	
	vecNrm(forw, forw);
	/* XXX When norming forward before the cross product we don't
	 * need to norm 'right' vector */
	vecNrm(left, left);

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

	camMov(cam, movVec, mov_trg);
}

void camMov(Camera *cam, Vec3 mov, int mov_trg) {
	vecAdd(cam->pos, mov, cam->pos);
	
	if(mov_trg) {
		vecAdd(cam->trg, mov, cam->trg);
	}

	camUpdPos(cam);
}

/* 
 * Update the position of a camera.
 *
 * @cam: Pointer to the camera to modify
*/
void camUpdPos(Camera *cam)
{
	camSetViewMat(cam, 
			cam->trg[0], cam->trg[1], cam->trg[2], 
			cam->pos[0], cam->pos[1], cam->pos[2]);
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
 * Create a view-matrix and write the
 * result into the specified vector.
 *
 * @cam: The camera to modify
 * @tx: The x-position of the target
 * @ty: The y-position of the target
 * @tz: The z-posiiton of the target
 * @px: The x-position of the camera
 * @py: The y-position of the camera
 * @pz: The z-position of the camera
 */
void camSetViewMat(Camera *cam, float tx, float ty, float tz, 
		float px, float py, float pz)
{
	Vec3 pos, trg, del, tmp, forw, left, up, stdup;

	/* The default up-vector */
	vecSet(stdup, 0.0, 1.0, 0.0);
	vecNrm(stdup, stdup);

	/* Set the target-position */
	vecSet(cam->trg, tx, ty, tz);
	vecCpy(trg, cam->trg);

	/* Set the camera-position */
	vecSet(cam->pos, px, py, pz);
	vecCpy(pos, cam->pos);

	/* The direction-vector from the camera to the target */
	vecSub(cam->pos, cam->trg, del);
	vecNrm(del, cam->dir);
	cam->dist = vecMag(del);

	vecSub(pos, trg, tmp);
	vecNrm(tmp, forw);
	
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

	cam->view[0xc]= -left[0] * pos[0] - left[1] * pos[1] - left[2] * pos[2];
	cam->view[0xd]= -up[0] * pos[0] - up[1] * pos[1] - up[2] * pos[2];
	cam->view[0xe]= -forw[0] * pos[0] - forw[1] * pos[1] - forw[2] * pos[2];
}
