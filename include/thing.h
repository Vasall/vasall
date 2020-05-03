#ifndef _THING_H_
#define _THING_H_

#include "object.h"

#define THING_LIMIT 128

/*
 * This struct is used as a wrapper
 * for interactable objects like barrels
 * and boxes. 
 */
struct thing_object {
	/*
	 * Pointer to the underlying object.
	 */
	struct object *obj;
};


/* The external thing-container */
extern struct ptr_list *thing_list;


/* Initialize the thing-array */
int thiInit(void);



#endif
