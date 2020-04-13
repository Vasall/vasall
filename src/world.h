#ifndef _WORLD_H_
#define _WORLD_H_

#include "global.h"
#include "model.h"

#define WORLD_SIZE 256

struct world {
	/*
	 * The x-size and the z-size
	 * of the world.
	 */
	int2_t size;
	int ptnum;


	/*
	 * The relative position of
	 * the terrain.
	 */
	vec3_t pos;

	/*  Both the absolute min- and max-positions */
	vec2_t min_pos, max_pos;

	/*
	 * The heightmap, containing
	 * the heights of each vertex
	 * of the terrain.
	 */
	float *heights;

	/*
	 * The relative rotation of
	 * the terrain, which will be
	 * passed to the model.
	 */
	vec3_t rot;

	/*
	 * The underlying model for
	 * the terrain.
	 */
	struct model *terrain;
};


/* The global world-struct */
extern struct world *world;


/* Create and initialize a new world */
int wldCreate(void);

/* Destroy a world */
void wldDestroy(void);

/* Render the world */
void wldRender(void);

/* Get the height of the terrain at the given position */
float wldGetHeight(float x, float z);

/* Generate the terrain of the world */
int wldGenTerrain(void);

#endif
