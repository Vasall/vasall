#ifndef _LIST_H_
#define _LIST_H_

struct ptr_list {
	/* The number of pointers in the array */
	int num;

	/* The number limit */
	int limit;

	/* The  */
	void **arr;
};

/* Create a new pointer-list */
struct ptr_list *lstCreate(int limit);

/* Destory a pointer-list */
void lstDestroy(struct ptr_list *lst);

/* Clear a pointer-list */
void lstClear(struct ptr_list *lst);

/* Insert a new pointer into the list */
int lstAdd(struct ptr_list *lst, void **ptr, int size);

/* Remove a pointer from the list */
void lstRemv(struct ptr_list *lst, void *ptr);

/* Reorder the pointers in the list */
void lstOrder(struct ptr_list *lst);

/* Display the list in the console */
void lstPrint(struct ptr_list *lst);

#endif
