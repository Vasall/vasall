#include "global.h"

uint8_t zero = 0;
uint8_t one = 1;

gloWrapper *core;

char *gloErrBuf = NULL;


/* 
 * Set a new error-message.
 *
 * @err: The string to set as the new
 * 	error message
*/
void gloSetError(const char *err)
{
	if(gloErrBuf == NULL) {
		gloErrBuf = (char *)malloc(128 * sizeof(char));
		if(gloErrBuf == NULL) {
			printf("Failed to set error-message.");
			return;
		}
	}

	strcpy(gloErrBuf, err);
}

/* 
 * Get the most recent error-message.
 *
 * Returns: The most recent error-message
*/
char *gloGetError(void)
{
	if(gloErrBuf == NULL) {
		return("No error-message set.");
	}

	return(gloErrBuf);
}


/*
 * Create a new wrapper and fill the
 * submodules with zeros.
 *
 * Returns: Either a pointer to the created wrapper
 * 	or NULL if an error occurred
*/
gloWrapper *gloCreate(void)
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
 * Destroy a global-wrapper and free the
 * allocated space. Note that this function
 * will just skip everything if the ptr is
 * NULL.
 *
 * @ptr: The pointer to the wrapper
*/
void gloDestroy(gloWrapper *ptr)
{
	if(ptr != NULL) {
		free(ptr);
	}
}
