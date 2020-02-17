#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "global.h"
#include "vec.h"
#include "mat.h"
#include "object.h"

typedef enum {LEFT, RIGHT, FORWARD, BACK} Direction;

struct camera {
	/*
	 * The current position of the
	 * camera in the world.
	*/
	Vec3 pos;

	/*
	 * The direction vectors for the camera,
	 * where forwards points in the direction,
	 * the camera is looking at, while right is
	 * left-vector to the forward-vector.
	 */
	Vec3 forward;
	Vec3 right;

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
	struct object *trg_obj;
};


/* The global camera-struct */
extern struct camera *camera;


/* Create a new camera and set the position */
int camCreate(float aov, float asp, float near, float far);

/* Destroy a camera */
void camDestroy(void);

/* Get the projection matrix of the camera */
void camGetProj(Mat4 mat);

/* Get the view matrix of the camera */
void camGetView(Mat4 mat);

/* Get the position of the camera */
void camGetPos(Vec3 pos);

/* Set the position of the camera */
void camSetPos(Vec3 pos);

/* Get the direction the camera is looking */
void camGetDir(Vec3 dir);

/* Set the camera-direction */
void camSetDir(Vec3 dir);

/* Change the rotation of the camera */
void camMouseMoved(int delx, int dely);

/* Adjust the zoom of the camera */
void camZoom(int val);

/* Rotates the camera, adjusting yaw and pitch */
void camRot(float d_yaw, float d_pitch);

/*  */
void camMovDir(Direction dir);

/* Moves the camera freely, when no target entity is set */
void camMov(Vec3 mov);

/* Create a new projection matrix */
void camSetProjMat(float aov, float asp, float near, float far);

/* Create a new view-matrix */
void camUpdViewMat(void);

/* Sets the cameras direction to look at a point */
void camLookAt(Vec3 trg);

/* Set the camera */
void camSet(Vec3 pos, Vec3 trg);

/* Set an object as the target-point */
void camTargetObj(struct object *obj);

/* Updates the camera */
void camUpdate(void);

#endif
