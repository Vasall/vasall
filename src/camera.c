#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "camera.h"

void setViewMat(float tx, float ty, float tz, float px, float py, float pz, Mat4 m);

/*
 * Create a new camera and initialize the
 * values of the struct with the default
 * settings and set the position of the
 * camera.
 *
 * @x: The x-position of the camera
 * @y: The y-position of the camera
 * @z: The z-position of the camera
 * 
 * Returns: Either the pointer to the created
 * 	camera-struct or null
 */
Camera *camCreate(int fx, int fy, int fz, float px, float py, float pz)
{
	float delx, dely, delz;
	Vec3 del;

	/* Initialize the camera-struct */
	Camera *cam;

	cam = malloc(sizeof(Camera));
	if(cam == NULL) {
		return(NULL);
	}
	memset(cam, 0, sizeof(Camera));

	/* Set the position of the target */
	cam->trg.x = fx;
	cam->trg.y = fy;
	cam->trg.z = fz;

	/* Set the position of the camera */
	cam->pos.x = px;
	cam->pos.y = py;
	cam->pos.z = pz;

	delx = px - fx;
	dely = py - fy;
	delz = pz - fz;
	del = vecCreate(delx, dely, delz);

	/* The normalized direction vector from the target to the camera */
	cam->dir = vecNrmRet(del);

	/* Calculate the distance between the target and the camera */
	cam->dist = vecMag(del);

	/* Set the sensitivity of the mouse */
	cam->sensitivity = 0.2;	

	/* Create the projection matrix */
	cam->proj = mat4Idt();
	setProjMat(45.0, (float)(800.0 / 600.0), 0.1, 1000.0, cam->proj);	

	cam->view = mat4Idt();
	setViewMat(fx, fy, fz, px, py, pz, cam->view);

	printf("Projection-matrix:\n");
	mat4Print(cam->proj);

	printf("View-matrix:\n");
	mat4Print(cam->view);

	cam->model = mat4Idt();
	printf("Model-matrix:\n");
	mat4Print(cam->model);

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

Mat4 camGetModel(Camera *cam)
{
	return(cam->model);
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

void camZoom(Camera *cam, int val)
{
	Vec3 vec;
	/* Calculate rotations (need to move in direction we're pointing) */
	vec.x = -sin(cam->rot.y * TO_RADS);
	vec.z = cos(cam->rot.y * TO_RADS);
	vec.y = sin(cam->rot.x * TO_RADS);

	vecNrm(&vec);
	printf("Normal: %f/%f/%f\n", vec.x, vec.y, vec.z);
	vecScl(&vec, val);
	printf("Before: %f/%f/%f\n", cam->pos.x, cam->pos.y, cam->pos.z);
	vecAdd(&cam->pos, vec);
	printf("Before: %f/%f/%f\n", cam->pos.x, cam->pos.y, cam->pos.z);
}


void movcam(Camera *cam, Direction dir) 
{
	/* Vector to add to our position */
	Vec3 movementVec = {0, 0, 0};

	/* Calculate rotations (need to move in direction we're pointing) */
	float cosXRot = cos(cam->rot.x * TO_RADS);
	float sinYRot = sin(cam->rot.y * TO_RADS);
	float cosYRot = cos(cam->rot.y * TO_RADS);

	float movVecLen;

	/* Move camera */
	if (dir == FORWARD) {
		movementVec.x += sinYRot * cosXRot;
		movementVec.z += -cosYRot * cosXRot;
	}
	else if (dir == BACK) {
		movementVec.x += -sinYRot * cosXRot;
		movementVec.z += cosYRot * cosXRot;
	}
	else if (dir == LEFT) {
		movementVec.x += -cosYRot;
		movementVec.z += -sinYRot;
	}
	else if (dir == RIGHT) {
		movementVec.x += cosYRot;
		movementVec.z += sinYRot;
	}

	/* Calculate length of vector */
	movVecLen = (float)sqrt(movementVec.x * movementVec.x + 
			movementVec.y * movementVec.y +
			movementVec.z * movementVec.z);

	/* Normalize vector */
	cam->pos.x += (movementVec.x / movVecLen) * 2;
	cam->pos.z += (movementVec.z / movVecLen) * 2;
}

void setProjMat(float aov, float asp, float near, float far, Mat4 m)
{
	float bottom, top, left, right, tangent;

	tangent = near * tan(aov * 0.5 * M_PI / 180);

	top = tangent;
	bottom = -top;
	right = top * asp;
       	left = -right; 

	printf("top/bottom: %.2f/%.2f\n", top, bottom);
	printf("right/left: %.2f/%.2f\n", right, left);

	m[0x0] = (2 * near) / (right - left);

	m[0x5] = (2 * near) / (top - bottom); 
	
	m[0x8] = (right + left) / (right - left); 
	m[0x9] = (top + bottom) / (top - bottom); 
	m[0xa] = -(far + near) / (far - near); 
	m[0xb] = -1; 
	
	m[0xe] = (-2 * far * near) / (far - near); 
	m[0xf] = 0;

}

void setViewMat(float fx, float fy, float fz, float px, float py, float pz, Mat4 m)
{
	Vec3 pos, trg, tmp, forw, left, up;
	
	tmp = vecCreate(0.0, 1.0, 0.0);

	trg = vecCreate(fx, fy, fz);
	pos = vecCreate(px, py, pz);

	printf("Target: %.2f/%.2f/%.2f\n", trg.x, trg.y, trg.z);
	printf("Position: %.2f/%.2f/%.2f\n", pos.x, pos.y, pos.z);

	forw = vecNrmRet(vecSubRet(pos, trg));
	left = vecNrmRet(vecCross(vecNrmRet(tmp), forw));
	up = vecCross(forw, left);

	m[0x0] = left.x;
	m[0x4] = left.y;
	m[0x8] = left.z;

	m[0x1] = up.x;
	m[0x5] = up.y;
	m[0x9] = up.z;

	m[0x2] = forw.x;
	m[0x6] = forw.y;
	m[0xa] = forw.z;

	m[0xc]= -left.x * pos.x - left.y * pos.y - left.z * pos.z;
	m[0xd]= -up.x * pos.x - up.y * pos.y - up.z * pos.z;
	m[0xe]= -forw.x * pos.x - forw.y * pos.y - forw.z * pos.z;
}
