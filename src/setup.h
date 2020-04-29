#ifndef _V_SETUP_H
#define _V_SETUP_H

#include "defines.h"
#include "core.h"

/* Create and configure the window */
V_API XSDL_Window *initWindow(void);

/* Initialize OpenGL */
V_API int initGL(void);

/* Load all resources into the differente caches */
V_API int loadResources(void);

/* Initialize UI-elements */
V_API int initUI(void);

#endif
