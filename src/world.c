#define NOT_DEFINE_WORLD
#include "world.h"

/*
 * Create and initialize the world-struct
 * and return the pointer to the struct.
 *
 * @x: The width of the world
 * @y: The height of the world
 * @l: The amount of layers
*/
extern vsWorld *wld_create(int x, int y, short l)
{
	int layer_size = (x * y);
	long tile_len = (layer_size * l);

	vsWorld *world = (vsWorld *)malloc(sizeof(vsWorld));
	if(world == NULL)
		return(NULL);

	/* Set size of the world */
	world->size.x = x;
	world->size.y = y;

	/* Set the amount of layers */
	world->layers = l;

	/* Initialize the flag-grid */
	world->grid = (uint8_t *)calloc(layer_size, sizeof(uint8_t));
	if(world->grid == NULL)
		return(NULL);

	/* Initialize the tile-grid */
	world->tiles = (vsTile *)malloc(sizeof(vsTile) * tile_len);
	if(world->tiles == NULL)
		return(NULL);

	return(world);
}
