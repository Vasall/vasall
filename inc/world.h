#ifndef _WORLD_H
#define _WORLD_H

#include "asset.h"
#include "model.h"

/* The side length of a single chunk in worldunits */
#define _CHUNK_SIZE 32


#define WLD_CHUNK_NUM 9

/*
 * A single chunk in the world.
 *
 * @pos: The upper-left corner in the world
 * @size: The size of the chunk in world-units
 */
struct world_chunk {
	uint32_t   id;

	vec2_t     pos;
	vec2_t     size;

	int        ptnum;
	float      *heights;


};


/*
 * The world-chunk-wrapper responsible for loading and managing the current
 * chunks.
 */
struct world_wrapper {
	int                 chunk_num;
	struct world_chunk  *chunks;
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
 * Load a chunk and push it into the chunk-buffer.
 *
 * @id: The id of the chunk to add
 * @buf: The buffer containing the chunk-data
 * @len: The length of the buffer in bytes
 */
extern int wld_load_chunk(uint32_t id, char *buf, int len);


/*
 * Remove and delete a chunk from the chunk-buffer. 
 *
 * @id: The id of the chunk to delete
 */
extern void wld_del_chunk(uint32_t id);


/*
 * Render the terrain and the static objects in the world.
 *
 * @interp: The interpolation-factor
 */
extern void wld_render(float interp);


/*
 * Get the height of the terrain at a certain position.
 *
 * @x: The x-position
 * @z: The z-position
 *
 * Returns: Either the height or 0 if an error occurred
 */
extern float wld_get_height(float x, float z);

#endif
