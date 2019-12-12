#ifndef VASALL_SETUP_H
#define VASALL_SETUP_H

#include "../enud/enud.h"

/* Create and configure the window */
ENUD_Window *initWindow(void);

/* Initialize OpenGL */
int initGL(void);

/* Load all resources into the differente caches */
int loadResources(void);

/* Initialize UI-elements */
int initUI(void);

#endif
