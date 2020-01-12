#ifndef VASALL_SETUP_H
#define VASALL_SETUP_H

#include "../XSDL/xsdl.h"

void try_login(XSDL_Node *n, XSDL_Event *e);

/* Create and configure the window */
XSDL_Window *initWindow(void);

/* Initialize OpenGL */
int initGL(void);

/* Load all resources into the differente caches */
int loadResources(void);

/* Initialize UI-elements */
int initUI(void);

#endif
