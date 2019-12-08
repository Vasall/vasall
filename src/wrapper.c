#include "wrapper.h"

uint8_t zero = 0;
uint8_t one = 1;

gloWrapper *core;


/*
 * Create a new wrapper and fill the
 * submodules with zeros.
 *
 * Returns: Either a pointer to the created wrapper
 * 	or NULL if an error occurred
*/
gloWrapper *initWrapper(void)
{
	gloWrapper *wrapper;

	wrapper = (gloWrapper *)malloc(sizeof(gloWrapper));
	if(wrapper == NULL) {
		perror("Failed to create global wrapper");
		return(NULL);
	}

	memset(wrapper, 0, sizeof(gloWrapper));

	/* Just to be save */
	wrapper->procevt = NULL;
	wrapper->update = NULL;
	wrapper->render = NULL;

	return(wrapper);
}

/*
 * Delete a wrapper-struct and free the
 * allocated space. Note that this function
 * will just skip everything if the ptr is
 * NULL.
 *
 * @ptr: The pointer to the wrapper
*/
void delWrapper(gloWrapper *ptr)
{
	if(ptr != NULL) {
		free(ptr);
	}
}
