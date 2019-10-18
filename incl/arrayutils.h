#ifndef _ARRAYUTILS_H_
#define _ARRAYUTILS_H_

/* ==== DEFINE PROTOTYPES ==== */
void merge(int *arr, int left, int mid, int right);
void merge_sort(int *arr, int left, int right);
void array_copy(int *src, int srcOff, int *dst, int dstOff, int len);

#endif
