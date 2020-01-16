#include "entity.h"
#include <stdlib.h>

/* Redefine external variables */
Player **player_array = NULL;
int player_number = 0;
Enemy **enemy_array = NULL;
int enemy_number = 0;
Thing **thing_array = NULL;
int thing_number = 0;

/* 
 * Initialize the entity-arrays.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int entInit(void)
{
	player_array = malloc(ENTITY_LIMIT * sizeof(Player *));
	if(object_array == NULL) return(-1);
	object_number = 0;

	enemy_array = malloc(ENTITY_LIMIT * sizeof(Enemy *));
	if(enemy_array == NULL) return(-1);
	enemy_number = 0;

	thing_array = malloc(ENTITY_LIMIT * sizeof(Thing *));
	if(thing_array == NULL) return(-1);
	thing_number = 0;

	return(0);
}

Player *plrCreate(Vec3 pos)
{
	Player *plr;

	/* Allocate memory for the struct */
	plr = malloc(sizeof(Player));
	if(plr == NULL) return(NULL);

	/* Create the underlying object */
	plr->obj = objCreate(pos);

	return(plr);
}



