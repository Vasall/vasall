#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

extern struct ptr_list *lstCreate(int limit)
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

extern void lstDestroy(struct ptr_list *lst)
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

extern void lstClear(struct ptr_list *lst)
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

extern int lstAdd(struct ptr_list *lst, void **ptr, int size)
{
	if(lst == NULL) return(-1);

	if(lst->num + 1 > lst->limit) return(-1);

	*ptr = malloc(size);
	if(*ptr == NULL) return(-1);

	lst->arr[lst->num] = *ptr;
	lst->num += 1;

	return(0);
}

extern void lstRemv(struct ptr_list *lst, void *ptr)
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

extern void lstOrder(struct ptr_list *lst)
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

extern void lstPrint(struct ptr_list *lst)
{
	int i;

	printf("Size: %d/%d\n", lst->num, lst->limit);
	for(i = 0; i < lst->limit; i++) printf("%d: %p\n", i, lst->arr[i]);
}

extern struct dyn_stack *stcCreate(int elesz)
{
	struct dyn_stack *stc = NULL;

	stc = malloc(sizeof(struct dyn_stack));
	if(stc == NULL) return(NULL);

	stc->num = 0;
	stc->alloc = 10;
	stc->size = elesz;

	stc->buf = malloc(stc->alloc * stc->size);
	if(stc->buf == NULL) {
		free(stc);
		return(NULL);
	}

	return(stc);
}

extern void stcDestroy(struct dyn_stack *stc)
{
	if(stc == NULL) return;

	/* Free the element-buffer */
	free(stc->buf);

	/* Free the struct-memory */
	free(stc);
}

extern int stcPush(struct dyn_stack *stc, void *in)
{
	void *ptr = NULL;

	if(stc->num + 1 > stc->alloc) {
		stc->alloc += 10;

		stc->buf = realloc(stc->buf, stc->alloc * stc->size);
		if(stc->buf == NULL) return(-1);
	}

	ptr = (uint8_t *)stc->buf + stc->num * stc->size;
	memcpy(ptr, in, stc->size);
	stc->num += 1;

	return(stc->num - 1);
}

extern int stcPull(struct dyn_stack *stc, void *out)
{
	void *ptr = NULL;

	if(stc->num < 1) return(-1);

	ptr = (uint8_t *)stc->buf + (stc->num - 1) * stc->size;
	memcpy(out, ptr, stc->size);
	stc->num -= 1;

	if(stc->num < stc->alloc - 10 && stc->alloc > 10) {
		stc->alloc -= 10;

		stc->buf = realloc(stc->buf, stc->alloc * stc->size);
		if(stc->buf == NULL) {
			return(-1);
		}
	}

	return(stc->num);
}

extern int stcCopy(struct dyn_stack *stc, int idx)
{
	uint8_t *ptr;
	int ret = -1;

	ptr = (uint8_t *)stc->buf + (idx * stc->size);
	ret = stcPush(stc, ptr);

	return(ret);
}

extern void *stcGet(struct dyn_stack *stc, int idx)
{
	uint8_t *ptr = NULL;

	if(idx >= stc->num || idx < 0) return(NULL);

	ptr = ((uint8_t *)stc->buf + (idx * stc->size));

	return(ptr);
}

extern void stcPrint(struct dyn_stack *stc)
{
	int i;
	uint8_t val;

	printf("Number: %d | Allocated: %d\n", stc->num, stc->alloc);

	for(i = 0; i < stc->num; i++) {
		printf("%d: ", i);
		val = *((uint8_t *)stc->buf + (i * stc->size));
		printf("%x ", val);
			
		printf("\n");
	}
}
