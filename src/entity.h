#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "object.h"

typedef struct Player {
	/*
	 * Pointer to the underlying object.
	 */
	Object *obj;

	/*
	 * The client-connection-info.
	 */
	struct sockaddr_in6 addr;
} Player;

/* The external player-container */
extern Player **players;

typedef struct Enemy {
	/*
	 * Pointer to the underlying object.
	 */
	Object *obj;

	/*
	 * 
	 */
} Enemy;

/* The external enemy-container */
extern Enemy **enemy;

typedef struct Thing {
	/*
	 * Pointer to the underlying object.
	 */
	Object *obj;
} Thing;

/* The external thing-container */
extern Thing **things;

#endif
