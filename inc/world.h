#ifndef _WORLD_H
#define _WORLD_H

#include "asset.h"
#include "model.h"

#define WORLD_SIZE 256

struct world_wrapper {
	int2_t size;
	int ptnum;
	vec2_t min_pos, max_pos;
	float *heights;
	vec3_t rot;
	short terrain;
};


/* Define the global world-wrapper instance */
extern struct world_wrapper world;


/*
 * Initialize the world, load the heightmap and generate the mesh for the
 * terrain.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int wld_init(void);


/*
 * Destroy the world and free the allocated memory.
 */
extern void wld_close(void);


/*
 * Render the terrain and the static objects in the world.
 */
extern void wld_render(void);


/*
 * Get the height of the terrain at a certain position.
 *
 * @x: The x-position
 * @z: The z-position
 *
 * Returns: Either the height or 0 if an error occurred
 */
extern float wld_get_height(float x, float z);


/*
 * Generate the heightmap.
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
extern int wld_gen_terrain(void);

#endif
