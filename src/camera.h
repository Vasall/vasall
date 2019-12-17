#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "vec.h"
#include "../enud/enud.h"

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
	 * The projection matrix.
	 */
	Mat4 proj;

	/*
	 * The current viewmatrix.
	 */
	Mat4 view;

	Mat4 model;
} Camera;

/* Create a new camera and set the position */
Camera *camCreate(int fx, int fy, int fz, float px, float py, float pz);

/* Destroy a camera */
void camDestroy(Camera *cam);

/* Get the projection matrix of the camera */
Mat4 camGetProj(Camera *cam);

/* Get the view matrix of the camera */
Mat4 camGetView(Camera *cam);

/* Get the position of the camera */
Vec3 camGetPos(Camera *cam);

Mat4 camGetModel(Camera *cam);

/* Get the direction the camera is looking */
Vec3 camGetDir(Camera *cam);

/* Change the rotation of the camera */
void camMouseMoved(Camera *cam, int delx, int dely);

void camZoom(Camera *cam, int val);

void movcam(Camera *cam, Direction dir);

/* Calculate a view matrix */
Mat4 lookAt(Vec3 from, Vec3 to);

/* Create a new projection matrix */
void setProjMat(float aov, float asp, float near, float far, Mat4 m);

#endif
