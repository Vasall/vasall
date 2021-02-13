#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


AMO_API void amo_arr_init(struct amo_array *arr, unsigned short size)
{
	if(arr == NULL)
		return;

	arr->size = size;
	arr->alloc = DYN_ALLOC_MIN;
	arr->num = 0;
}


AMO_API void amo_arr_free(struct amo_array *arr)
{
	if(arr == NULL || arr->buf == NULL)
		return;

	free(arr->buf);
}


AMO_API int amo_arr_push(struct amo_array *arr, void *ptr, unsigned short num)
{
	if(arr == NULL)
		return -1;

	/* Allocate the array if necessary */
	if(arr->buf == NULL) {
		if(!(arr->buf = malloc(arr->size * arr->alloc)))
			return -1;
	}

	/* Reallocate buffer if necessary */
	if((arr->num + num) >= arr->alloc) {
		void *p = NULL;

		arr->alloc *= 1.5;

		if(!(p = realloc(arr->buf, arr->alloc * arr->size)))
			return -1;

		arr->buf = p;
	}

	/* Copy data into the array */
	memcpy(arr->buf + (arr->num * arr->size), ptr, num * arr->size);

	/* Increment number of elements in array */
	arr->num += num;
	return num * arr->size;
}


AMO_API int amo_arr_get(struct amo_array *arr, void *out, unsigned short start, 
		unsigned short end)
{
	unsigned int off;
	unsigned int range;

	if(arr == NULL || arr->buf == NULL || out == NULL)
		return -1;

	off = start * arr->size;
	range = (end - start) * arr->size;

	memcpy(out, arr->buf + off, range);

	return range;
}


AMO_API int amo_arr_fit(struct amo_array *arr)
{
	char *p;

	if(arr == NULL || arr->buf == NULL)
		return -1;

	if(!(p = realloc(arr->buf, arr->size * arr->num)))
		return -1;

	arr->buf = p;
	return 0;
}
