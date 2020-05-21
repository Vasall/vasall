#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include <stdint.h>


/*
 * Allocate a buffer and write the content of a file into the buffer. Note 
 * that after the returned buffer has been used, the memory has to be freed
 * to prevent memory leaks.
 *
 * @pth: The path to the file to load into the buffer
 * @buf: A pointer to attach the allocated memory to
 * @len: The length of the buffer in bytes
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
extern int fs_load_file(char *pth, uint8_t **buf, long *len);


/*
 * Allocate a buffer and write the png-file into the buffer. Note that after
 * the returned buffer has been used, the memory has to be freed, to prevent
 * memory leaks.
 *
 * @pth: The path to the file
 * @buf: A pointer to attach the allocated memory to
 * @w: The with of the image
 * @h: The height of the image
 */
extern int fs_load_png(char *pth, uint8_t **buf, int *w, int *h);

#endif
