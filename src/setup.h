#ifndef VASALL_SETUP_H
#define VASALL_SETUP_H

#include "../enud/enud.h"

/* Initialize the core structs */
int init(int argc, char **argv);

/* Initialize OpenGL */
int initGL(void);

/* Create and configure the window */
ENUD_Window *initWindow(void);

/* Load all resources into the differente caches */
int loadResources(void);

/* Initialize UI-elements */
int initUI(void);

#endif
