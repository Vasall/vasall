#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "object.h"

#define PLAYER_LIMIT 3

/*
 * This struct is used as a wrapper
 * for player-objects, which is needed
 * to store the client-information of 
 * the other player.
 */
struct player_object {
	/*
	 * Pointer to the underlying object.
	 */
	struct object *obj;
};


/* The external player-container */
extern struct ptr_list *player_list;


/* Initialize the player-array */
int plrInit(void);

/* Destroy the player-array */
void plrClose(void);

/* Create a new player */
struct player_object *plrCreate(Vec3 pos);

/* Remove a player from the player-array */
void plrDelete(struct player_object *plr);

#endif
