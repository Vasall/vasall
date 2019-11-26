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
	world->w = x;
	world->h = y;

	printf("%d - %d\n", world->w, world->h);

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

	int ix, iy;
	for(ix = 0; ix < world->w; ix++) {
		for(iy = 0; iy < world->h; iy++) {
			vsTile *tile = (vsTile *)&world->tiles[ix + (iy * world->h)];
			if(rand() % 100 > 80) {
				tile->index = 1;
			}
			else {
				tile->index = 0;
			}
		}
	}

	return(world);
}

/*
 * Render the world.
 */
extern int wld_render(ENUD_Renderer *ren, ENUD_Camera *cam, 
		vsWorld *world)
{
	int rw, rh;
	ENUD_ConvUn(cam, 1, 1, &rw, &rh);

	int ox, oy;
	ENUD_GetPos(g_context, cam, 0, 0, &ox, &oy);

	int x, y;
	for(x = 0; x < world->w; x++) {
		for(y = 0; y < world->h; y++) {
			vsTile *tile = (vsTile *)&world->tiles[x + (y * world->h)];
			ENUD_RenderSprite(ren, 
					g_context, cam,
					ENUD_Sprites[0], 
					tile->index, 
					x + 0.5,
					y + 0.5);
		}
	}

	return(0);
}

