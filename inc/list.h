#ifndef _LIST_H
#define _LIST_H

struct ptr_list {
	int num;
	int limit;
	void **arr;
};

extern struct ptr_list *lstCreate(int limit);
extern void lstDestroy(struct ptr_list *lst);
extern void lstClear(struct ptr_list *lst);

extern int lstAdd(struct ptr_list *lst, void **ptr, int size);
extern void lstRemv(struct ptr_list *lst, void *ptr);

extern void lstOrder(struct ptr_list *lst);
extern void lstPrint(struct ptr_list *lst);


struct dyn_stack {
	int num;
	int alloc;
	int size;
	void *buf;
};

extern struct dyn_stack *stcCreate(int elesz);
extern void stcDestroy(struct dyn_stack *stc);

extern int stcPush(struct dyn_stack *stc, void *in);
extern int stcPull(struct dyn_stack *stc, void *out);

extern int stcCopy(struct dyn_stack *stc, int idx);
extern void *stcGet(struct dyn_stack *stc, int idx);
extern void stcPrint(struct dyn_stack *stc);

#endif
