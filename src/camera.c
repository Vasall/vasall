#include "camera.h"

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
Camera *camCreate(int x, int y, int z)
{
	/* Initialize the camera-struct */
	Camera *cam;

	cam = (Camera *)malloc(sizeof(Camera));
	if(cam == NULL) {
		return(NULL);
	}
	memset(cam, 0, sizeof(Camera));

	/* Set the position of the mouse */
	cam->pos.x = x;
	cam->pos.y = y;
	cam->pos.z = z;

	/* Set the sensitivity of the mouse */
	cam->sensitivity = 0.2;

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
