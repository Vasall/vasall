#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "vec.h"
#include "mat.h"
#include "object.h"
#include "XSDL/xsdl.h"

typedef enum {LEFT, RIGHT, FORWARD, BACK} Direction;

typedef struct Camera  {
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
	Mat4 proj;

	/*
	 * The current viewmatrix.
	 */
	Mat4 view;

	/*
	 * The target entity. When this is not NULL, the camera will
	 * always follow this entity
	 */
	Object *trg_obj;

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

/* Set the position of the camera */
void camSetPos(Camera *cam, Vec3 pos);

/* Get the direction the camera is looking */
void camGetDir(Camera *cam, Vec3 dir);

/* Change the rotation of the camera */
void camMouseMoved(Camera *cam, int delx, int dely);

/* Adjust the zoom of the camera */
void camZoom(Camera *cam, int val);

/* Rotates the camera, adjusting yaw and pitch */
void camRot(Camera *cam, float d_yaw, float d_pitch);

/*  */
void camMovDir(Camera *cam, Direction dir);

/* Moves the camera freely, when no target entity is set */
void camMov(Camera *cam, Vec3 mov);

/* Create a new projection matrix */
void camSetProjMat(Camera* cam, float aov, float asp, 
		float near, float far);

/* Create a new view-matrix */
void camUpdViewMat(Camera *cam);

/* Sets the cameras direction to look at a point */
void camLookAt(Camera *cam, Vec3 trg);

/* Set the camera */
void camSet(Camera *cam, Vec3 pos, Vec3 trg);

#endif
