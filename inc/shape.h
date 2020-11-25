#ifndef _SHAPE_H
#define _SHAPE_H

#include "vector.h"

/*
 * The following shapes are viewed as modifed base-shapes (ie. unit-sphere with
 * a radius of 1, or blender cube with a sidelength of 2) with both a position
 * and a scaling factor.
 */

typedef struct shape_cube {
	vec3_t pos;
	vec3_t scl;
} cube_t;

typedef struct shape_sphere {
	vec3_t pos;
	vec3_t scl;
} sphere_t;

#endif
