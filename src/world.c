#include "world.h"
#include "loader.h"
#include "../enud/enud.h"

float heightMap[WORLD_SIZE][WORLD_SIZE];
float faceNormals[WORLD_SIZE][WORLD_SIZE][3];
float materialColours[WORLD_SIZE][WORLD_SIZE][4];

/*
 * Initialize a default world-struct.
 *
 * Returns: Either a pointer to the created
 * 	struct or NULL if an error occurred
*/
World *initWorld(void)
{
	World *world;

	world = (World *)malloc(sizeof(World));
	if(world == NULL) {
		return(NULL);
	}

	srand(13);
	generateTerrain(world);

	return(world);
}

/*
 * Generate a new terrain and write it
 * to the world-struct.
 *
 * @world: Pointer to the world to create a
 * 	terrain for
*/
void generateTerrain(World *world)
{   
	int x, z;
	float tmp;
	float** heightmapImage;

	if(world){/* Prevent warning for not using world */}

	/* Reset heightmap */
	memset(heightMap, 0, (WORLD_SIZE * WORLD_SIZE));

	heightmapImage = loadPPMHeightmap("../res/images/heightmap_256.ppm", 1, WORLD_SIZE);

	/* Iterate over all points in heightmap (not incl. water)*/
	for(x = 0; x < WORLD_SIZE; x++) {
		for(z = 0; z < WORLD_SIZE; z++) {

			/* Set height */
			heightMap[x][z] = 40 * heightmapImage[x][z] + 10;
			/*heightMap[x][z] = 10;*/
			tmp = heightMap[x][z];
			/*printf("%d/%d:%f\n", x, z, tmp);*/

			/* Grass */
			if (heightMap[x][z] <= 15) {
				materialColours[x][z][0] = 0.0;
				materialColours[x][z][1] = 0.3;
				materialColours[x][z][2] = 0.2;
			}
			/* Grass to dirt transition */
			else if (tmp > 15 && tmp <= 20) {
				materialColours[x][z][0] = 
					(0.0 + ((tmp - 20) * 0.1));
				materialColours[x][z][1] = 
					(0.3 - ((tmp - 20) * 0.01));
				materialColours[x][z][1] = 
					(0.2 - ((tmp - 20) * 0.02));
			}
			/* Dirt */
			else if (tmp > 20 && tmp <= 30) {
				materialColours[x][z][0] = 0.52;
				materialColours[x][z][1] = 0.26;
				materialColours[x][z][2] = 0.08;
			}
			/* Dirt to snow transition */
			else if (tmp > 30 && tmp <= 35) {
				materialColours[x][z][0] = 
					(0.52 + ((tmp - 30) * 0.1));
				materialColours[x][z][1] = 
					(0.26 + ((tmp - 30) * 0.15));
				materialColours[x][z][2] = 
					(0.08 + ((tmp - 30) * 0.18));
			}
			/* Snow */
			else {
				materialColours[x][z][0] = 1;
				materialColours[x][z][1] = 1;
				materialColours[x][z][2] = 1;
			}
			materialColours[x][z][3] = 1;
		}
	}

	smoothTerrain(world, 0.2);
	calculateFaceNormals(world);
}


/*
 * Smooth out the terrain.
 *
 * @world: The world to smooth out the terrain for
 * @smooth: The factpr used to smooth out
*/
void smoothTerrain(World *world, float smooth) 
{  
	int x, z;

	if(world){/* Prevent warning for not using world */}

	if (smooth < 0) {
		smooth = 0;
	}
	if (smooth > 0.9) {
		smooth = 0.9;
	}

	/* rows, left to right */
	for(x = 1; x < WORLD_SIZE; x++) {
		for(z = 0; z < WORLD_SIZE; z++) {
			heightMap[x][z] = heightMap[x - 1][z] * smooth +
				heightMap[x][z] * (1 - smooth);
		}
	}

	/* rows, right to left */
	for(x = WORLD_SIZE - 1; x > -1; x--) {
		for(z = 0; z < WORLD_SIZE; z++) {
			heightMap[x][z] = heightMap[x + 1][z] * smooth + 
				heightMap[x][z] * (1 - smooth);
		}
	}

	/* columns, bottom to top */
	for(x = 0; x < WORLD_SIZE; x++) {
		for(z = 1; z < WORLD_SIZE; z++) {
			heightMap[x][z] = heightMap[x][z - 1] * smooth + 
				heightMap[x][z] * (1 - smooth);
		}
	}

	/* columns, top to bottom */
	for(x = 0; x < WORLD_SIZE; x++) {
		for(z = WORLD_SIZE - 1; z > -1; z--) {
			heightMap[x][z] = heightMap[x][z + 1] * smooth + 
				heightMap[x][z] * (1 - smooth);
		}
	}
}

/*
 * Draw a world using OpenGL.
 *
 * @world: A pointer to the world to draw
*/
void renderTerrain(World *world) 
{    
	int x, z;
	float terrainOffset = (WORLD_SIZE) / 2.0;

	if(world){/* Prevent warning for not using world */}

	/* Iterate over all values in heightmap */
	for(x = 0; x < WORLD_SIZE - 1; x++) {
		glBegin(GL_QUAD_STRIP);
		for(z = 0; z < WORLD_SIZE; z++) {
			glMaterialfv(GL_FRONT, GL_AMBIENT, materialColours[x][z]);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, materialColours[x][z]);

			glNormal3fv(faceNormals[x + 1][z]);
			glVertex3f(x + 1 - terrainOffset, heightMap[x + 1][z], 
					z - terrainOffset);

			glNormal3fv(faceNormals[x][z]);
			glVertex3f(x - terrainOffset, heightMap[x][z], 
					z - terrainOffset);

		}
		glEnd();
	}
}

void calculateFaceNormals(World *world)
{    
	int x, z;

	if(world){/* Prevent warning for not using world */}

	/* Calculate normals */
	for(x = 0; x < WORLD_SIZE; x++) {
		for(z = 0; z < WORLD_SIZE; z++) {
			float vx, vy, vz, len;
			float t1[3], t2[3], t3[3];
			float v1[3], v2[3];	

			/* x, z */
			t1[0] = x; 
			t1[1] = heightMap[x][z]; 
			t1[2] = z;

			/* x + 1, z */
			t3[0] = x+1; 
			t3[1] = heightMap[x+1][z]; 
			t3[2] = z;

			/* x, z + 1 */
			t2[0] = x; 
			t2[1] = heightMap[x][z+1]; 
			t2[2] = z+1;

			v1[0] = t2[0] - t1[0];
			v1[1] = t2[1] - t1[1];
			v1[2] = t2[2] - t1[2];

			v2[0] = t3[0] - t1[0];
			v2[1] = t3[1] - t1[1];
			v2[2] = t2[2] - t1[2];

			vx = v1[1] * v2[2] - v1[2] * v2[1];
			vy = v1[2] * v2[0] - v1[0] * v2[2];
			vz = v1[0] * v2[1] - v1[1] * v2[0];

			len = (float)sqrt(vx * vx + vy * vy + vz * vz);

			faceNormals[x][z][0] = (vx / len);
			faceNormals[x][z][1] = (vy / len);
			faceNormals[x][z][2] = (vz / len);
		}
	}
}

float getHeight(World *world, float x, float z) 
{   
	float A, B, C, D;
	float xPercent, zPercent, h;

	/* Coordinate (0,0) corresponds with middle of terrain map */
	float xIndexInHeightmap = x + (WORLD_SIZE / 2.0);
	float zIndexInHeightmap = z + (WORLD_SIZE / 2.0);

	if(world){/* Prevent warning for not using world */}

	/* If outside of terrain, set height to 0 */
	if(xIndexInHeightmap < 0 || xIndexInHeightmap >= 
			WORLD_SIZE - 1) {
		return(3);
	}
	if(zIndexInHeightmap < 0 || zIndexInHeightmap >= 
			WORLD_SIZE - 1) {
		return(3);
	}
	if(heightMap[(int)floor(xIndexInHeightmap)]
			[(int)floor(zIndexInHeightmap)] < 1) {
		return(1);
	}

	/* B(0,1) ------ C(1,1) */
	/*   |      pos    |    */
	/* A(0,0) ------ D(1,0) */
	A = heightMap[(int)floor(xIndexInHeightmap)]
		[(int)floor(zIndexInHeightmap)];
	B = heightMap[(int)floor(xIndexInHeightmap)]
		[(int)floor(zIndexInHeightmap + 1)];
	C = heightMap[(int)floor(xIndexInHeightmap+1)]
		[(int)floor(zIndexInHeightmap + 1)];
	D = heightMap[(int)floor(xIndexInHeightmap + 1)]
		[(int)floor(zIndexInHeightmap)];

	/* Calculate percent position is along x and z, in [0,1] */
	xPercent = xIndexInHeightmap - floor(xIndexInHeightmap);
	zPercent = zIndexInHeightmap - floor(zIndexInHeightmap);

	/* Calculate height, interpolating between 4 points surrounding current pos */
	/* This formula is from wikipedia.com/Bilinear_interpolation */
	h = (A * (1 - xPercent) * (1 - zPercent)) + 
		(D * (xPercent) * (1 - zPercent));
	h += (B * (1 - xPercent) * (zPercent)) +
		(C * (xPercent) * (zPercent));

	return (h);
}

float *getNormal(World *world, float x, float z) 
{
	/* Coordinate (0,0) corresponds with middle of terrain map */
	int xIndex = x + (WORLD_SIZE / 2.0);
	int zIndex = z + (WORLD_SIZE / 2.0);

	if(world){/* Prevent warning for not using world */}

	/* If outside of terrain, return null */
	if(xIndex < 0 || xIndex >= WORLD_SIZE - 1) {
		return(NULL);
	}
	if(zIndex < 0 || zIndex >= WORLD_SIZE - 1) {
		return(NULL);
	}

	return(faceNormals[xIndex][zIndex]);
}
