#include "arrayutils.h"

/**
 * Helper Method to merge the two sorted subarrays together.
 * @param {arr} the array to merge, consisting of two already sorted subarrays
 * @param {left} the left border of the first subarray
 * @param {mid} the middle, splitting arr into two subarrays
 * @param {right} the right border of the second subarray
 */
void merge(int arr[], int left, int mid, int right) {  
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
  
    // Copy the remaining elements of L[], if there are any
    while (i < n1) {
        arr[k] = l[i]; 
        i++;
        k++;
    } 
  
    // Copy the remaining elements of R[], if there are any
    while (j < n2) {
        arr[k] = r[j]; 
        j++;
        k++;
    } 
} 
  
/**
 * Sorts the subarray of arr with mergesort
 * @param {arr} the input array
 * @param {left} left index of subarray to be sorted (inclusive)
 * @param {right} right index of subarray to be sorted (inclusive)
 */
void mergeSort(int arr[], int left, int right) {
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
