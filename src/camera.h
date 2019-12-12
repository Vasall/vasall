#ifndef VASALL_CAMERA_H
#define VASALL_CAMERA_H

#include "vec.h"
#include "../enud/enud.h"

typedef enum {LEFT, RIGHT, FORWARD, BACK} Direction;

typedef struct Camera  {
	/*
	 * The current position of the
	 * camera in the world.
	*/
	Vec3 pos;

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
} Camera;

/* Create a new camera and set the position */
Camera *camCreate(int x, int y, int z);

/* Destroy a camera */
void camDestroy(Camera *cam);

/* Change the rotation of the camera */
void camMouseMoved(Camera *cam, int delx, int dely);

void camZoom(Camera *cam, int val);

void movcam(Camera *cam, Direction dir);


#endif
