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
 	Vec3 del;

	/* Initialize the camera-struct */
	Camera *cam;

	cam = calloc(1, sizeof(Camera));
	if(cam == NULL) {
		return(NULL);
	}

	/* Set the position of the target */
	cam->trg.x = 0;
	cam->trg.y = 0;
	cam->trg.z = 0;

	/* Set the position of the camera */
	cam->pos.x = 1.0;
	cam->pos.y = 1.0;
	cam->pos.z = 1.0;

	/* Get the connecting vector between the camera and the target */
	del = vecSubRet(cam->pos, cam->trg);

	/* The normalized direction vector from the target to the camera */
	cam->dir = vecNrmRet(del);

	/* Calculate the distance between the target and the camera */
	cam->dist = vecMag(del);

	/* Set the sensitivity of the mouse */
	cam->sensitivity = 0.2;	

	/* Create the projection matrix */
	cam->proj = mat4Idt();
	camSetProjMat(cam, aov, asp, near, far);

	/* Create the view-matrix */
	cam->view = mat4Idt();
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
 * Get the projection matrix of a camera.
 *
 * @cam: Pointer to the camera
 *
 * Returns: A 4x4 matrix representing
 * 	the projection matrix
 */
Mat4 camGetProj(Camera *cam)
{
	return(cam->proj);
}

/*
 * Get the view matrix of a camera.
 *
 * @cam: Pointer to the camera
 *
 * Returns: A 4x4 matrix representing
 * 	the view matrix
 */
Mat4 camGetView(Camera *cam)
{
	return(cam->view);
}

/* 
 * Get the position of the camera.
 *
 * @cam: Pointer to the camera
 *
 * Returns: A 3d-vector containing the
 * 	position of the camera
 */
Vec3 camGetPos(Camera* cam)
{
	return(cam->pos);
}

/* 
 * Get the direction the camera is looking.
 * This function will return the normalvector
 * pointing from the camera to the target.
 *
 * @cam: Pointer to the camera
 *
 * Returns: A 3d-vector containing the
 * 	direction the camera is pointing
 * 	towards
 *
 */
Vec3 camGetDir(Camera *cam)
{
	Vec3 dir;
	vecCpy(&dir, &cam->dir);
	vecNrm(&dir);
	return(dir);
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
	cam->rot.x += ((float)dely) * cam->sensitivity;
	cam->rot.y += ((float)delx) * cam->sensitivity;

	/* Limit looking down to vertically down */
	if (cam->rot.x > 90) {
		cam->rot.x = 90;
	}
	if (cam->rot.x < -90) {
		cam->rot.x = -90;
	}

	/* Modulo of int is not available */
	if (cam->rot.y < 0) {
		cam->rot.y += 360;
	}
	if (cam->rot.y > 360) {
		cam->rot.y += -360;
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
	cam->dist += val;

	camUpdPos(cam);
}


void camMovDir(Camera *cam, Direction dir) 
{
	/* Vector to add to our position */
	Vec3 movVec = {0, 0, 0};

	Vec3 up = vecCreate(0.0, 1.0, 0.0);
	Vec3 forward = vecCreate(cam->dir.x, 0.0, cam->dir.z);
	Vec3 right = vecCross(up, forward);
	
	vecNrm(&forward);
	/* XXX When norming forward before the cross product we don't
	 * need to norm 'right' vector */
	vecNrm(&right);

	/* UP and DOWN not yet implemented */

	switch(dir) {
		case FORWARD:
			movVec = forward;
			break;
		case BACK:
			movVec = vecSclRet(forward, -1.0);
			break;
		case RIGHT:
			movVec = right;
			break;
		case LEFT:
			movVec = vecSclRet(right, -1.0);
			break;
		default:
			break;
	}

	movcam(cam, movVec);
}

void camMov(Camera *cam, Vec3 mov) {
	vecAdd(&cam->pos, mov);
	camUpdPos(cam);
}

/* 
 * Update the position of a camera.
 *
 * @cam: Pointer to the camera to modify
*/
void camUpdPos(Camera *cam)
{
	cam->pos = vecSclRet(cam->dir, cam->dist);

	camSetViewMat(cam, 
			cam->trg.x, cam->trg.y, cam->trg.z, 
			cam->pos.x, cam->pos.y, cam->pos.z);
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
 * @m: the matrix to write the result to
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
 * @fx: The x-position of the target
 * @fy: The y-position of the target
 * @fz: The z-posiiton of the target
 * @px: The x-position of the camera
 * @py: The y-position of the camera
 * @pz: The z-position of the camera
 * @m: The 4x4 matrix to write the result to
 */
void camSetViewMat(Camera *cam, float fx, float fy, float fz, 
		float px, float py, float pz)
{
	Vec3 pos, trg, del, tmp, forw, left, up;

	tmp = vecCreate(0.0, 1.0, 0.0);

	cam->trg = trg = vecCreate(fx, fy, fz);
	cam->pos = pos = vecCreate(px, py, pz);

	del = vecSubRet(cam->pos, cam->trg);
	cam->dir = vecNrmRet(del);
	cam->dist = vecMag(del);

	forw = vecNrmRet(vecSubRet(pos, trg));
	left = vecNrmRet(vecCross(vecNrmRet(tmp), forw));
	up = vecCross(forw, left);

	cam->view[0x0] = left.x;
	cam->view[0x4] = left.y;
	cam->view[0x8] = left.z;

	cam->view[0x1] = up.x;
	cam->view[0x5] = up.y;
	cam->view[0x9] = up.z;

	cam->view[0x2] = forw.x;
	cam->view[0x6] = forw.y;
	cam->view[0xa] = forw.z;

	cam->view[0xc]= -left.x * pos.x - left.y * pos.y - left.z * pos.z;
	cam->view[0xd]= -up.x * pos.x - up.y * pos.y - up.z * pos.z;
	cam->view[0xe]= -forw.x * pos.x - forw.y * pos.y - forw.z * pos.z;
}
