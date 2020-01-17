#include "thing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Redefine external variables */
struct ptr_list *thing_list = NULL;

/* 
 * Initialize the thing-array and
 * allocate necessary memory.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int thiInit(void)
{
	thing_list = lstCreate(THING_LIMIT);
	if(thing_list == NULL) return(-1);

	return(0);
}

