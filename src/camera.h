#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "vec.h"
#include "mat.h"
#include "XSDL/xsdl.h"

typedef enum {LEFT, RIGHT, FORWARD, BACK} Direction;

typedef struct Camera  {
	/*
	 * The point on the map currently 
	 * focused by the camera. This
	 * position is also used to calculate
	 * the position of the camera.
	 */
	Vec3 trg;

	/*
	 * The current position of the
	 * camera in the world.
	*/
	Vec3 pos;

	/*
	 * The vector representing the
	 * current direction of the camera.
	 * Note taht this vector is flipped.
	 * To actually get the direction of the
	 * camera, scale the vector by -1.
	 */
	Vec3 dir;

	/*
	 * The distance between the target and
	 * the camera.
	 */
	float dist;

	/*
	 * The current rotation of the
	 * camera in degrees.
	*/
	Vec3 rot;

	/*
	 * The sensivity of the camera, which
	 * influences how the camera can
	 * be controlled by the mouse.
	*/
	float sensitivity;

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
	Mat4 proj;

	/*
	 * The current viewmatrix.
	 */
	Mat4 view;

} Camera;

/* Create a new camera and set the position */
Camera *camCreate(float aov, float asp, float near, float far);

/* Destroy a camera */
void camDestroy(Camera *cam);

/* Get the projection matrix of the camera */
void camGetProj(Camera *cam, Mat4 mat);

/* Get the view matrix of the camera */
void camGetView(Camera *cam, Mat4 mat);

/* Get the position of the camera */
void camGetPos(Camera *cam, Vec3 pos);

/* Get the direction the camera is looking */
void camGetDir(Camera *cam, Vec3 dir);

/* Change the rotation of the camera */
void camMouseMoved(Camera *cam, int delx, int dely);

/* Adjust the zoom of the camera */
void camZoom(Camera *cam, int val);

void camMovDir(Camera *cam, Direction dir, int mov_trg);

void camMov(Camera *cam, Vec3 mov, int mov_trg);

/* Update the position of a camera */
void camUpdPos(Camera *cam);

/* Create a new projection matrix */
void camSetProjMat(Camera* cam, float aov, float asp, 
		float near, float far);

/* Create a new view-matrix */
void camSetViewMat(Camera *cam, float fx, float fy, float fz, 
		float px, float py, float pz);

#endif
