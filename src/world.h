#ifndef VASALL_WORLD_H
#define VASALL_WORLD_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../ENUD/enud.h"
#include "vector.h" 
#include "global.h"

typedef struct vsTile {
	/*
	 * The position of this tile.
	*/
	int x;
	int y;

	/*
	 * Index of the tile in the
	 * tile-cache.
	*/
	int index;

	/*
	 * Additional data attached to this
	 * specific tile. This is used to 
	 * store things, like spritestate,
	 * resources or others.
	*/
	void *data;
} vsTile;

typedef struct vsField {

} vsField;

typedef struct vsWorld {
	/*
	 * The dimensions of the world
	 * as a 2d-array. The layer-amount
	 * is defined seperatelly.
	*/
	int w;
	int h;

	/*
	 * The amount of layers, this 
	 * world consists of.
	*/
	short layers;
	
	/*
	 * The state of the different tiles,
	 * ie if it's passable or unpassable.
	 * Each tile can have up to 8 flags,
	 * as it contains 8 bits. 
	*/
	uint8_t *grid;

	/*
	 * The tile-array containing all
	 * tiles in all layers.
	*/
	vsTile *tiles;
} vsWorld;

/*
 * Create an empty world-struct and
 * return the pointer to the memory,
 * the world has been placed at.
*/
extern vsWorld *wld_create(int x, int y, short l);

/*
 * Delete a world and free the
 * allocated memory.
*/
extern int wld_delete(vsWorld *world);

/*
 * Render a world.
*/
extern int wld_render(ENUD_Renderer *ren, ENUD_Camera *cam, vsWorld *world);

#endif
