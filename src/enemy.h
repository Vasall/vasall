#ifndef _ENEMY_H_
#define _ENEMY_H_

#include "object.h"

#define ENEMY_LIMIT 64

/*
 * This struct is used as a wrapper
 * for enemy-objects, which is needed
 * to store the next actions of the 
 * enemy.
 */
struct enemy_object {
	/*
	 * Pointer to the underlying object.
	 */
	struct object *obj;

};


/* The external enemy-container */
extern struct ptr_list *enemy_list;


/* Initialize the enemy-array */
int enmInit(void);

#endif
