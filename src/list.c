#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

/* 
 * Create a new dynamic stack and allocate
 * memory for the struct.
 *
 * elesz: The size of a single element in the stack
 *
 * Returns: Either a pointer to the created stack
 * 	or NULL if an error occurred
 */
struct dyn_stack *stcCreate(int elesz)
{
	struct dyn_stack *stc;

	stc = malloc(sizeof(struct dyn_stack));
	if(stc == NULL) return(NULL);

	stc->num = 0;
	stc->alloc = 10;
	stc->ele_size = elesz;

	stc->buf = malloc(stc->alloc * stc->ele_size);
	if(stc->buf == NULL) {
		free(stc);
		return(NULL);
	}

	return(stc);
}

/* 
 * Destroy a dynamic stack and free the
 * allocated memory.
 *
 * @stc: Pointer to the stack to destroy
 */
void stcDestroy(struct dyn_stack *stc)
{
	free(stc->buf);
	free(stc);
}

/* 
 * Push a new element to the end of a stack.
 *
 * @stc: Pointer to the stack
 * @in: Pointer to the element to push into the stack
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int stcPush(struct dyn_stack *stc, void *in)
{
	void *ptr;

	if(stc->num + 1 > stc->alloc) {
		stc->alloc += 10;

		stc->buf = realloc(stc->buf, stc->alloc * stc->ele_size);
		if(stc->buf == NULL) {
			return(-1);
		}
	}

	ptr = (uint8_t *)stc->buf + stc->num * stc->ele_size;
	memcpy(ptr, in, stc->ele_size);
	stc->num += 1;

	return(0);
}

/* 
 * Pull the last element of a stack and write it
 * to the given buffer.
 *
 * @stc: Pointer to the stack
 * @out: Pointer to the memory to write the element to
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int stcPull(struct dyn_stack *stc, void *out)
{
	void *ptr;

	if(stc->num < 1) return(-1);

	ptr = (uint8_t *)stc->buf + (stc->num - 1) * stc->ele_size;
	memcpy(out, ptr, stc->ele_size);
	stc->num -= 1;

	if(stc->num < stc->alloc - 10 && stc->alloc > 10) {
		stc->alloc -= 10;

		stc->buf = realloc(stc->buf, stc->alloc * stc->ele_size);
		if(stc->buf == NULL) {
			return(-1);
		}
	}

	return(0);
}

/*
 * Print both the size of the stack and
 * output all element in the array.
 *
 * @stc: Pointer to the stack to display
 */
void stcPrint(struct dyn_stack *stc)
{
	int i;
	unsigned char val;

	printf("Number: %d | Allocated: %d\n", stc->num, stc->alloc);

	for(i = 0; i < stc->num; i++) {
		printf("%d: ", i);
		val = *((unsigned char *)stc->buf + (i * stc->ele_size));
		printf("%x ", val);
			
		printf("\n");
	}
}
