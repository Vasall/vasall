#include "global.h"

/* Redefine external variables */
uint8_t zero = 0;
uint8_t one = 1;
gloWrapper *core = NULL;
char gloErrBuf[256];

/* 
 * Set a new error-message.
 *
 * @err: The string to set as the new
 * 	error message
*/
void gloSetError(char *err)
{	
	strcpy(gloErrBuf, err);
}

/* 
 * Get the most recent error-message.
 *
 * Returns: The most recent error-message
*/
char *gloGetError(void)
{
	return(gloErrBuf);
}


/*
 * Initialize the global-wrapper.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
*/
int gloInit(void)
{
	core = calloc(1, sizeof(gloWrapper));
	if(core == NULL) {
		gloSetError("Failed to create global wrapper");
		return(-1);
	}

	/* Just to be save */
	core->procevt = NULL;
	core->update = NULL;
	core->render = NULL;

	/* Initialize the object-array */
	if(objInit() < 0) {
		gloSetError("Failed to initialize object-array");
		return(-1);
	}

	/* Initialize the different entity-arrays */
	if(entInit() < 0) {
		gloSetError("Failed to initialize entity-arrays");
		return(-1);
	}

	return(0);
}

/*
 * Destroy a global-wrapper and free the
 * allocated space. Note that this function
 * will just skip everything if the ptr is
 * NULL.
 *
 * @ptr: The pointer to the wrapper
*/
void gloClose(void)
{
	if(core == NULL) return;

	free(core);
}
