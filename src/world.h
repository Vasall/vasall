#ifndef _WORLD_H_
#define _WORLD_H_

#include "model.h"
#include "../enud/enud.h"

#define WORLD_SIZE 256

typedef struct Field {
	char *fid;

	int x;
	int y;

	GLuint vao;
	GLuint vbo;

	float *heights;
} Field;

typedef struct World {
	int width;
	int height;
	int size;

	float *heightmap;

	Model *model;
} World;

/* Create and initialize a new world */
World *wldCreate(void);

/* Destroy a world */
void wldDestroy(World *world);

/* Generate the terrain of the world */
int wldGenTerrain(World *world);

/* Render the terrain of the world */
void renderTerrain(World *world);

/* Get the height of the terrain at the given position */
float getHeight(World *world, float x, float z);
float *getNormal(World *world, float x, float z);

void calculateFaceNormals(World *world);

#endif
