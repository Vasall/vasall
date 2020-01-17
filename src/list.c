#include "list.h"
#include <stdio.h>
#include <stdlib.h>

/* 
 * Create a new pointer-list and set
 * each element to NULL.
 *
 * @limit: The pointer-limit for this list
 *
 * Returns: Either a pointer to the created
 * 	list or NULL if an error occurred
 */
struct ptr_list *lstCreate(int limit)
{
	int i;
	struct ptr_list *lst;

	if(limit < 1) return(NULL);

	lst = malloc(sizeof(struct ptr_list));
	if(lst == NULL) return(NULL);

	lst->limit = limit;
	lst->num = 0;

	lst->arr = malloc(limit * sizeof(void *));
	if(lst->arr == NULL) {
		free(lst);
		return(NULL);
	}

	for(i = 0; i < limit; i++) {
		lst->arr[i] = NULL;
	}

	return(lst);
}

/* 
 * Destory a pointer-list, free all pointers
 * and then free the whole struct-pointer.
 *
 * @lst: Pointer to the list-struct
 */
void lstDestroy(struct ptr_list *lst)
{
	int i;

	if(lst == NULL) return;

	for(i = 0; i < lst->limit; i++) {
		if(lst->arr[i] != NULL) {
			free(lst->arr[i]);
		}
	}

	free(lst);
}

/* 
 * Clear a pointer-list and free all allocated
 * pointers.
 *
 * @lst: Pointer to the list-struct
 */
void lstClear(struct ptr_list *lst)
{
	int i;

	if(lst == NULL) return;

	for(i = 0; i < lst->limit; i++) {
		if(lst->arr[i] != NULL) {
			free(lst->arr[i]);
			lst->arr[i] = NULL;
		}
	}
}

/* 
 * Allocated the given amount of bytes and add the
 * pointer to the list.
 *
 * @lst: Pointer to the list to insert pointer into
 * @ptr: The pointer to add to the list
 * @size: The amount of bytes to allocate
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int lstAdd(struct ptr_list *lst, void **ptr, int size)
{
	if(lst == NULL) return(-1);

	if(lst->num + 1 > lst->limit) return(-1);

	*ptr = malloc(size);
	if(*ptr == NULL) return(-1);

	lst->arr[lst->num] = *ptr;
	lst->num += 1;

	return(0);
}

/* 
 * Remove a pointer from the list, free the allocated 
 * memory and set the slot to NULL. Then reorder the 
 * left elements, to fill in empty holes in the list. 
 *
 * @lst: Pointer to remove the pointer from
 * @ptr: The pointer to remove
 */
void lstRemv(struct ptr_list *lst, void *ptr)
{
	int i;

	if(lst == NULL) return;

	for(i = 0; i < lst->num; i++) {
		if(lst->arr[i] == ptr) {
			free(lst->arr[i]);
			lst->arr[i] = NULL;
			lst->num -= 1;
			lstOrder(lst);
			break;
		}
	}
}

/* 
 * Reorder the pointers in the list.
 *
 * @lst: Pointer to the list to reorder 
 */
void lstOrder(struct ptr_list *lst)
{
	int i;
	
	for(i = 1; i < lst->limit; i++) {
		if(i != 0 && lst->arr[i - 1] == NULL && lst->arr[i] != NULL) {
			printf("%d\n", i);
			lst->arr[i - 1] = lst->arr[i];
			lst->arr[i] = NULL;
			i = 1;
		}
	}	
}

/* 
 * Display the list in the console.
 *
 * @lst: Pointer to the list
 */
void lstPrint(struct ptr_list *lst)
{
	int i;

	printf("Size: %d/%d\n", lst->num, lst->limit);
	for(i = 0; i < lst->limit; i++) printf("%d: %p\n", i, lst->arr[i]);
}
