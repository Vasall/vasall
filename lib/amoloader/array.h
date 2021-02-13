#ifndef _AMO_ARRAY_H
#define _AMO_ARRAY_H

#include "define.h"

#define DYN_ALLOC_MIN 20

struct amo_array {
	/* The size of one single element in bytes */
	unsigned short size;

	/* The number of elements in the array */
	unsigned short num;

	/* The number of allocated slots for elements */
	unsigned short alloc;

	/* The buffer containing the data */
	char *buf;
};


/*
 * Initialize a new dynamic array and allocated the first few slots of the
 * array.
 *
 * @arr: Pointer to the dynamic-array-struct to initialize
 * @size: The size of a single element in the array
 *
 * Returns: 0 on success or -1 if an error occurred
 */
AMO_API void amo_arr_init(struct amo_array *arr, unsigned short size);


/*
 * Free the memory used by the array.
 */
AMO_API void amo_arr_free(struct amo_array *arr);


/*
 * Push one or more elements into the array.
 *
 * @arr: Pointer to the array to push the array into
 * @ptr: Pointer-address to start copying data from
 * @num: The number of elements to copy from the address into the array
 *
 * Returns: Return the number of bytes copied or -1 if an error occurred
 */
AMO_API int amo_arr_push(struct amo_array *arr, void *ptr, unsigned short num);


/*
 * Copy elements in a given range from the array to a given address. 
 *
 * @arr: Pointer to the array to copy data from
 * @out: Address to copy the data to
 * @start: The index of the element to start reading from
 * @end: The index of the element to end reading (will be read aswell)
 *
 * Returns: The number of bytes copied or -1 if an error occurred
 */
AMO_API int amo_arr_get(struct amo_array *arr, void *out, unsigned short start, 
		unsigned short end);


/*
 * Resize the allocated buffer to fit the number of elements.
 *
 * @arr: Pointer to the array
 *
 * Returns: 0 on success or -1 if an error occurred
 */
AMO_API int amo_arr_fit(struct amo_array *arr);



#endif
