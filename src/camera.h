#ifndef VASALL_CAMERA_H
#define VASALL_CAMERA_H

#include "../enud/enud.h"

#define TO_RADS 3.141592654/180.0

typedef enum {LEFT, RIGHT, FORWARD, BACK} Direction;

typedef struct Camera  {
    float pos[3];
    float rot[3];

    int midWinX;
    int midWinY;


    float movementSpeedFactor;
    float mouseSensitivity;
} Camera;

/* Create a new camera */
Camera *initCamera(void);

/* Change the rotation of the camera */
void mouseMoved(Camera *cam, int deltaX, int deltaY);


void movcam(Camera *cam, Direction dir);


#endif
