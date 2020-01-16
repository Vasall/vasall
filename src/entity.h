#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include "object.h"

#define ENTITY_LIMIT 32

/*
 * This struct is used as a wrapper
 * for player-objects, which is needed
 * to store the client-information of 
 * the other player.
 */
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

/*
 * This struct is used as a wrapper
 * for enemy-objects, which is needed
 * to store the next actions of the 
 * enemy.
 */
typedef struct Enemy {
	/*
	 * Pointer to the underlying object.
	 */
	Object *obj;
} Enemy;

/*
 * This struct is used as a wrapper
 * for interactable objects like barrels
 * and boxes. 
 */
typedef struct Thing {
	/*
	 * Pointer to the underlying object.
	 */
	Object *obj;
} Thing;


/* The external player-container */
extern Player **player_array;
extern int player_number;

/* The external enemy-container */
extern Enemy **enemy_array;
extern int enemy_number;

/* The external thing-container */
extern Thing **thing_array;
extern int thing_number;


/* Initialize the entity-arrays */
int entInit(void);

/* Create a new player */
Player *plrCreate(Vec3 pos);

#endif
