#include "camera.h"

/*
 * Create a new camera and initialize the
 * values of the struct.
 * 
 * Returns: Either the pointer to the created
 *  camera-struct or null
 */
Camera *initCamera(void)
{
	/* Initialize the camera-struct */
	Camera *cam;

	cam = (Camera *)malloc(sizeof(Camera));
	if(cam == NULL) {
		return(NULL);
	}

	memset(cam, 0, sizeof(Camera));

	cam->pos[0] = 0;
	cam->pos[1] = 50;
	cam->pos[2] = 0;

	cam->midWinX = 0;
	cam->midWinY = 0;

	cam->movementSpeedFactor = 0.2;
	cam->mouseSensitivity = 0.2;

	return(cam);
}


void mouseMoved(Camera *cam, int deltaX, int deltaY)
{
	/* Rotate camera */
	cam->rot[0] += ((float) deltaY) * cam->mouseSensitivity;
	cam->rot[1] += ((float) deltaX) * cam->mouseSensitivity;

	/* Limit looking down to vertically down */
	if (cam->rot[0] > 90) {
		cam->rot[0] = 90;
	}
	if (cam->rot[0] < -90) {
		cam->rot[0] = -90;
	}

	if (cam->rot[1] < 0) {
		cam->rot[1] += 360;
	}
	if (cam->rot[1] > 360) {
		cam->rot[1] += -360;
	}
}


void movcam(Camera *cam, Direction dir) 
{
	/* Vector to add to our position */
	float movementVec[3] = {0, 0, 0};

	/* Calculate rotations (need to move in direction we're pointing) */
	float cosXRot = cos(cam->rot[0] * TO_RADS);
	float sinYRot = sin(cam->rot[1] * TO_RADS);
	float cosYRot = cos(cam->rot[1] * TO_RADS);

	float movVecLen;

	/* Move camera */
	if (dir == FORWARD) {
		movementVec[0] += sinYRot * cosXRot;
		movementVec[2] += -cosYRot * cosXRot;
	}
	else if (dir == BACK) {
		movementVec[0] += -sinYRot * cosXRot;
		movementVec[2] += cosYRot * cosXRot;
	}
	else if (dir == LEFT) {
		movementVec[0] += -cosYRot;
		movementVec[2] += -sinYRot;
	}
	else if (dir == RIGHT) {
		movementVec[0] += cosYRot;
		movementVec[2] += sinYRot;
	}

	/* Calculate length of vector */
	movVecLen = (float)sqrt(movementVec[0] * movementVec[0] + 
			/*movementVec[1] * movementVec[1] + */
			movementVec[2] * movementVec[2]);

	/* Normalize vector */
	cam->pos[0] += (movementVec[0] / movVecLen) * cam->movementSpeedFactor;
	cam->pos[2] += (movementVec[2] / movVecLen) * cam->movementSpeedFactor;
}
