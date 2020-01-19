#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include "global.h"

/* Redefine external variables */
struct input_binds *input = NULL;


/*
 * Initialize the input-binds and allocate
 * the necessary memory.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int inpInit(void)
{
	input = malloc(sizeof(struct input_binds));
	if(input == NULL) return(-1);

	return(0);
}

/*
 * Close the input-binds and free the 
 * allocated memory.
 */
void inpClose(void)
{
	free(input);
}
