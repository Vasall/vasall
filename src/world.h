#ifndef _WORLD_H_
#define _WORLD_H_

#include "model.h"
#include "XSDL/xsdl.h"

#define WORLD_SIZE 256

struct world {
	/*
	 * The x-size and the z-size
	 * of the world. 
	 */
	int xsize;
	int zsize;
	int ptnum;

	/*
	 * The heightmap, containing
	 * the heights of each vertex
	 * of the terrain.
	 */
	float *heights;

	/*
	 * The relative position of
	 * the terrain, which will be
	 * passed to the model.
	 */	
	Vec3 pos;

	/*
	 * The relative rotation of
	 * the terrain, which will be
	 * passed to the model.
	 */
	Vec3 rot;

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

/* Generate the terrain of the world */
int wldGenTerrain(void);

/* Render the world */
void wldRender(void);

/* Get the height of the terrain at the given position */
float wldGetHeight(float x, float z);

#endif
