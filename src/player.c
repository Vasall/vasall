#include "player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Redefine external variables */
struct ptr_list *player_list = NULL;

/* 
 * Initialize the player-array and
 * allocate the necessary memory.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int plrInit(void)
{
	player_list = lstCreate(PLAYER_LIMIT);
	if(player_list == NULL) return(-1);

	return(0);
}

/* 
 * Destroy the player-array and delete
 * all player-instances. Then free the
 * allocated memory.
 */
void plrClose(void)
{
	lstDestroy(player_list);
}

/* 
 * Create a new player and initialize
 * the instance with the default values.
 *
 * @pos: The position to spawn the player at
 *
 * Returns: Either a pointer to the created
 * 	player-instance or NULL if an error
 * 	occurred
 */
struct player_object *plrCreate(Vec3 pos)
{
	struct player_object *plr;

	lstAdd(player_list, (void **)&plr, sizeof(struct player_object));
	if(plr == NULL) goto failed;

	memset(plr, 0, sizeof(struct player_object));

	/* Create the underlying object */
	plr->obj = objCreate(pos);
	if(plr->obj == NULL) goto failed;
	
	return(plr);

failed:
	return(NULL);
}

/*
 * Remove a player from the player-array
 * and free the allocated spot. Then update
 * the slots in the player-array.
 *
 * @plr: Pointer to the player to remove
 */
void plrDelete(struct player_object *plr)
{
	if(plr){}
}
