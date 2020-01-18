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


struct dyn_stack {
	/* The number of elements in the array */
	int num;

	/* The number of allocated element-slots */
	int alloc;

	/* The size of one single element in the array */
	int ele_size;

	/* The data-buffer */
	void *buf;
};

/* Create a new dynamic stack */
struct dyn_stack *stcCreate(int elesz);

/* Destroy a dynamic stack */
void stcDestroy(struct dyn_stack *stc);

/* Push a new element to the end of a stack */
int stcPush(struct dyn_stack *stc, void *in);

/* Pull the last element of a stack */
int stcPull(struct dyn_stack *stc, void *out);

/* Print all elements in the stack */
void stcPrint(struct dyn_stack *stc);

#endif
