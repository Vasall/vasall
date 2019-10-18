#include "arrayutils.h"

/**
 * Helper Method to merge the two sorted subarrays together.
 *
 * @arr: The array to merge, consisting of two already sorted subarrays
 * @left: The left border of the first subarray
 * @mid: The middle, splitting arr into two subarrays
 * @right: The right border of the second subarray
 */
void merge(int *arr, int left, int mid, int right) {  
    int n1 = mid - left + 1; 
    int n2 =  right - mid; 
  
    // create temp arrays
    int l[n1], r[n2]; 
  
    // Copy data to temp arrays
    for (int i = 0; i < n1; i++) 
        l[i] = arr[left + i];
    for (int j = 0; j < n2; j++) 
        r[j] = arr[mid + j + 1];
  
    // Merge the temp arrays back into arr[l..r] 
    int i = 0;	// Initial index of first subarray
    int j = 0;	// Initial index of second subarray 
    int k = left;	// Initial index of merged subarray 
    while(i < n1 && j < n2) { 
        if (l[i] <= r[j])
            arr[k] = l[i]; 
        else
            arr[k] = r[j];
        i++;	
	j++;
        k++;
    } 
  
    // Copy the remaining elements of l[], if there are any
    while (i < n1) {
        arr[k] = l[i]; 
        i++;
        k++;
    } 
  
    // Copy the remaining elements of r[], if there are any
    while (j < n2) {
        arr[k] = r[j]; 
        j++;
        k++;
    } 
} 
  
/**
 * Sorts subarray of an array with mergesort.
 *
 * @arr: the input array
 * @left: left index of subarray to be sorted (inclusive)
 * @right: Right index of subarray to be sorted (inclusive)
 */
void merge_sort(int *arr, int left, int right) {
    if (left < right) {
        // Find the middle of our subarray
        int mid = (left + right) >> 1;
  
        // Sort first and second halves 
        mergeSort(arr, left, mid); 
        mergeSort(arr, mid + 1, right); 

		// Merge them together
        merge(arr, left, mid, right); 
    } 
} 

/**
 * Copys part of one array to somewhere in another array.
 *
 * @src: The source array
 * @srcOff: The offset from start of source array
 * @dst: The destination array
 * @dstOff: The offset from start of destination array
 * @len: The length of subarray to copy
 */
void array_copy(int *src, int srcOff, int *dst, int dstOff, int len) {
	for(int i = 0; i < len; i++) {
		dst[dstOff + i * sizeof(int)] = src[srcOff + i * sizeof(int)];
	}
}
