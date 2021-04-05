#ifndef _WORLD_H
#define _WORLD_H

#include "asset.h"
#include "model.h"

/* The side length of a single chunk in worldunits */
#define _CHUNK_SIZE 64


#define WLD_CHUNK_LIM 9

#define WLD_CHUNK_OBJ_LIM 32

#define WLD_COL_AXIS_X 0
#define WLD_COL_AXIS_Y 1
#define WLD_COL_AXIS_Z 2

#define WLD_COL_MIN (0<<31)
#define WLD_COL_MAX (1<<31)

/*
 * A single collision-representing an object in the world, which is used for the
 * broadphase-collision-detection.
 *
 * @data: Containing the box owner id and the min-max-flag
 * @value: The min or max value
 */
struct world_col_point {
	unsigned int    data;
	float           value;
};

struct world_col_box {
	short    obj;

	cube_t   box;

	short    min[3];
	short    max[3];

	short    colobj[5];
};

struct light_buffer {
	vec3_t sun;
};


/*
 * The world-chunk-wrapper responsible for loading and managing the current
 * chunks.
 */
struct world_wrapper {
	vec2_t                  pos;
	vec2_t                  size;

	/* SAP-collision */
	struct world_col_point  col_point[3][2*WLD_CHUNK_OBJ_LIM];
	struct world_col_box    col_box[WLD_CHUNK_OBJ_LIM];

	/* Temporary world-model TODO */
	short mdl;

	/* Light buffer */
	unsigned int light_bo;
	struct vk_buffer light_buf;
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

#if 0
/*
 * 
 * Add a new collision-box to the world.
 *
 * @obj: The index of the object in the object-table
 * @pos: The current position of the collision-box
 * @box: The collision-box
 * @idx: A pointer to write the index of the collision-box to
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int wld_col_add(short obj, vec3_t pos, cube_t box, short *idx);


/*
 * Move a collision-box to a new position.
 *
 * @slot: The slot of the collision-box in the list
 * @pos: The new position
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int wld_col_move(short slot, vec3_t pos);


/*
 * Sort the endpoints and update the overlapping collision-boxes.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int wld_col_update(void);
#endif

#endif
