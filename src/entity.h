#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "object.h"

#define ENTITY_LIMIT 32

typedef struct Player {
	/*
	 * Pointer to the underlying object.
	 */
	Object *obj;

#ifdef DEBUG
	/*
	 * The client-connection-info.
	 */
	struct sockaddr_in6 addr;
#endif
} Player;

/* The external player-container */
extern Player **player_array;
extern int player_number;

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
extern Enemy **enemy_array;
extern int enemy_number;

typedef struct Thing {
	/*
	 * Pointer to the underlying object.
	 */
	Object *obj;
} Thing;

/* The external thing-container */
extern Thing **thing_array;
extern int thing_number;

/* Initialize the entity-arrays */
int entInit(void);

/* Create a new player */
Player *plrCreate(Vec3 pos);

#endif
