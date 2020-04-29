#ifndef _V_MODEL_UTILS_H
#define _V_MODEL_UTILS_H

#include "defines.h"
#include "model.h"

/*
 * Loads a model from a wavefront .obj file. The model gets added to the model
 * cache and the index is returned.
 *
 * @mdl: The model struct to set the mash of
 * @pth: The absolute path to the obj-file
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
V_API int mdl_load_obj(char *pth, int *idxnum, int **idx, int *vtxnum,
		vec3_t **vtx, vec3_t **nrm, vec2_t **uv);

#endif
