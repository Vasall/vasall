#ifndef _V_INPUT_H
#define _V_INPUT_H

#include "defines.h"

struct input_map {
	float camera[2];

	float movement[2];
};

V_GLOBAL struct input_map *inp_map;

/* 
 * Initialize the input-map.
 *
 * Returns: Either 0 on succes or -1
 * 	if an error occurred
 */
V_API int inp_init(void);

/*
 * Close the input-handler and free the allocated memory.
 */
V_API void inp_close(void);

#endif
