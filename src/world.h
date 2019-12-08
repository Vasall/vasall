#ifndef VASALL_WORLD_H
#define VASALL_WORLD_H

#define WORLD_SIZE 256

extern float heightMap[WORLD_SIZE][WORLD_SIZE];
extern float faceNormals[WORLD_SIZE][WORLD_SIZE][3];
extern float materialColours[WORLD_SIZE][WORLD_SIZE][4];

typedef struct World {
	int i;
} World;

/* Initialize the world */
World *initWorld(void);

void generateTerrain(World *world);
void renderTerrain(World *world);
float getHeight(World *world, float x, float z);
float *getNormal(World *world, float x, float z);

void generateWater(World *world, float);
void smoothTerrain(World *world, float);
void calculateFaceNormals(World *world);

#endif
