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

	return(world);
}

extern int wld_render(ENUD_Renderer *ren, vsCamera *cam, vsWorld *world)
{
	int rw, rh;
	cam_convun(cam, 1, 1, &rw, &rh);

	int ox, oy;
	cam_getpos(cam, 0, 0, &ox, &oy);

	int x, y;
	for(x = 0; x < world->w; x++) {
		for(y = 0; y < world->h; y++) {
			ENUD_Rect rect = {
				(ox + (rw * x)),
				(oy + (rh * y)),
				rw,
				rh
			};

			ENUD_Sprite *spr = ENUD_Sprites[0];

			ENUD_RenderCopy(ren,
					spr->frames[0],
					NULL,
					&rect);
		}
	}
}

