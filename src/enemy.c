#include "enemy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Redefine external variables */
struct ptr_list *enemy_list = NULL;

/*
 * Initialize the enemy array and
 * allocate the necessary memory.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int enmInit(void)
{
	enemy_list = lstCreate(ENEMY_LIMIT);
	if(enemy_list == NULL) return(-1);

	return(0);
}

