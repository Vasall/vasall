#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Redefine external-variables */
struct input_map *inp_map = NULL;

/* 
 * Initialize the input-map.
 *
 * Returns: Either 0 on succes or -1
 * 	if an error occurred
 */
int inpInit(void)
{
	inp_map = calloc(1, sizeof(struct input_map));
	if(inp_map == NULL) return(-1);

	return(0);
}
