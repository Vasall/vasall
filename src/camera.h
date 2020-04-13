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
	struct object *trg_obj;
};


/* The global camera-struct */
extern struct camera *camera;


/* Create a new camera and set the position */
int camCreate(float aov, float asp, float near, float far);

/* Destroy a camera */
void camDestroy(void);

/* Get the projection matrix of the camera */
void camGetProj(mat4_t mat);

/* Get the view matrix of the camera */
void camGetView(mat4_t mat);

/* Get the position of the camera */
void camGetPos(vec3_t pos);

/* Set the position of the camera */
void camSetPos(vec3_t pos);

/* Get the direction the camera is looking */
void camGetDir(vec3_t dir);

/* Set the camera-direction */
void camSetDir(vec3_t dir);

/* Change the rotation of the camera */
void camMouseMoved(int delx, int dely);

/* Adjust the zoom of the camera */
void camZoom(int val);

/* Rotates the camera, adjusting yaw and pitch */
void camRot(float d_yaw, float d_pitch);

/*  */
void camMovDir(Direction dir);

/* Moves the camera freely, when no target entity is set */
void camMov(vec3_t mov);

/* Create a new projection matrix */
void camSetProjMat(float aov, float asp, float near, float far);

/* Create a new view-matrix */
void camUpdViewMat(void);

/* Sets the cameras direction to look at a point */
void camLookAt(vec3_t trg);

/* Set the camera */
void camSet(vec3_t pos, vec3_t trg);

/* Set an object as the target-point */
void camTargetObj(struct object *obj);

/* Updates the camera */
void camUpdate(void);

#endif
