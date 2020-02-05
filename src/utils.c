#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * This function will open a file and then
 * read it's content into an allocated buffer.
 *
 * @pth: The absolute path to the file
 * @buf: An unallocated ptr
 * @len: A pointer to write the length of the buffer to
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int readFile(char *pth, uint8_t **buf, long *len)
{
	FILE *fd;
	long length;
	uint8_t *data;

	/* Open the file */
	fd = fopen(pth, "rb");
	if(fd == NULL) return(-1);

	/* Get the length of the file */
	fseek(fd, 0, SEEK_END);
	length = ftell(fd);
	
	/* Allocate a buffer */
	data = malloc(length);
	if(data == NULL) {
		fclose(fd);
		return(0);
	}

	/* Read the content into the buffer */
	fseek(fd, 0, SEEK_SET);
	fread(data, length, 1, fd);

	/* Close the file */
	fclose(fd);

	/* Set the output-variables */
	*buf = data;
	*len = length;

	return(0);
}
