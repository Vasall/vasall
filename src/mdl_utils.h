#ifndef _MDL_UTILS_H_
#define _MDL_UTILS_H_

#define BUF_ALLOC_STEP 1024

#include "model.h"

/* Load a obj-file into a model-struct */
int mdlLoadObj(struct model *mdl, char *pth);

#endif
