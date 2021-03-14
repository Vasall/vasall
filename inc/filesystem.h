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

/*
 * Write the content of a buffer into a file, overwriting any previoulsy stored
 * data inside this file or create a new file.
 * 
 * @pth: The path to the file
 * @buf: A pointer to a buffer containing the data, which will be written to the
 *       file or NULL
 * @length: The length of the buffer. If buf is NULL, length must be 0
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
extern int fs_write_file(char *pth, uint8_t *buf, long length);

/*
 * Create a new directory.
 * 
 * @pth: The path to the directory
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
extern int fs_create_dir(char *pth);

#endif
