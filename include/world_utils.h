#ifndef _V_WORLD_UTILS_H
#define _V_WORLD_UTILS_H

#include "defines.h"

#include <stdio.h>
#include <stdint.h>

/*
 * This function is used to calculate the number of vertices needed for a
 * terrain of the size vtxnum*vtxnum.
 *
 * @vtxnum: The side-length of the terrain-square
 *
 * Returns: The number of vertices
 */
V_API int calcVertexNum(int vtxnum);

/*
 * Generate a buffer containing the ordered indices for a square terrain
 * of the length vtxnum * vtxnum.
 *
 * @vtxnum: The side-length of the terrain-square
 * @idxlen: A pointer to write the number of indices to
 *
 * Returns: An array containing the ordered indices or NULL if an 
 * 	error occurred
 */
V_API unsigned int *genIndexBuf(int vtxnum, int *idxlen);

#endif
