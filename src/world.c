#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include "world.h"
#include "global.h"
#include "stdutil.h"
#include "shader.h"
#include "loader.h"
#include "../enud/enud.h"

float delF = 0.0;

static int storeQuad1(uint32_t *idxbuf, int ptr, uint32_t tl, uint32_t tr, 
		uint32_t bl, uint32_t br, int8_t mixed) {
	idxbuf[ptr++] = tl;
	idxbuf[ptr++] = bl;
	idxbuf[ptr++] = mixed ? tr : br;
	idxbuf[ptr++] = br;
	idxbuf[ptr++] = tr;
	idxbuf[ptr++] = mixed ? bl : tl;
	return(ptr);
}

static int storeQuad2(uint32_t *idxbuf, int ptr, uint32_t tl, uint32_t tr, 
		uint32_t bl, uint32_t br, int8_t mixed) {
	idxbuf[ptr++] = tr;
	idxbuf[ptr++] = tl;
	idxbuf[ptr++] = mixed ? br : bl;
	idxbuf[ptr++] = bl;
	idxbuf[ptr++] = br;
	idxbuf[ptr++] = mixed ? tl : tr;
	return (ptr);
}

/* 
 * Generate an array containing indices for all
 * buffers.
 */
static unsigned int *genIdxBuf(int num, int *len)
{
	int idxc = ((num - 1) * (num - 1)) * 6;
	int ptr, col, row;
	unsigned int *idxbuf = malloc(idxc * sizeof(unsigned int));

	*len = idxc;

	ptr = 0;
	for(col = 0; col < num - 1; col++) {
		for(row = 0; row < num - 1; row++) {
			uint32_t topLeft = twodim(col, row, num);
			uint32_t topRight = twodim(col + 1, row, num);
			uint32_t bottomLeft = twodim(col, row + 1, num);
			uint32_t bottomRight = twodim(col + 1, row + 1, num);

			if(row % 2 == 0) {
				ptr = storeQuad1(idxbuf, ptr, 
						topLeft, topRight,
						bottomLeft, bottomRight,
						col % 2 == 0);
			}
			else {
				ptr = storeQuad2(idxbuf, ptr,
						topLeft, topRight,
						bottomLeft, bottomRight,
						col % 2 == 0);
			}
		}
	}

	return(idxbuf);
}

/*
 * Initialize a default world-struct.
 *
 * Returns: Either a pointer to the created
 * 	struct or NULL if an error occurred
 */
World *wldCreate(void)
{
	World *world;

	/* Allocate space for the world-struct */
	world = (World *)malloc(sizeof(World));
	if(world == NULL) {
		return(NULL);
	}

	/* Allocate space for the heightmap */
	world->width = WORLD_SIZE;
	world->height = WORLD_SIZE;
	world->size = (WORLD_SIZE * WORLD_SIZE);
	world->heightmap = (float *)malloc((world->size) * 
			sizeof(float));
	if(world->heightmap == NULL) {
		return(NULL);
	}

	memset(world->heightmap, 0, world->size);

	world->model = NULL;

	/* Generate the terrain */
	wldGenTerrain(world);

	return(world);
}

/*
 * Delete a world and free allocated memory.
 *
 * @world: Pointer to the world
 * 	to delete
 */
void wldDestroy(World *world)
{
	/* Free the heightmap  */
	free(world->heightmap);

	/* Free the struct itself */
	free(world);
}

/*
 * Generate a new terrain and write it
 * to the world-struct.
 *
 * @world: Pointer to the world to create a
 * 	terrain for
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int wldGenTerrain(World *world)
{   
	int x, z, w, idx, vsz;
	float tmp;
	float **heightmapImage;
	float *heightmap;
	Vertex *vertices;
	ColorRGB *colors;
	uint32_t *indices;
	int indlen;
	Model *mdl;	

	heightmap = world->heightmap;
	w = world->width;

	/* Reset heightmap */
	memset(heightmap, 0, (WORLD_SIZE * WORLD_SIZE));

	/* Initialize the vertex-array */
	vsz = world->size;
	vertices = malloc(vsz * sizeof(Vertex));
	if(vertices == NULL) {
		printf("Failed to allocate space for vertices.\n");
		return(-1);
	}


	/* Create an array for all vertice-colors */
	colors = malloc(vsz * sizeof(ColorRGB));
	if(colors == NULL) {
		printf("Failed to allocate space for colors.\n");
		return(-1);
	}

	heightmapImage = loadPPMHeightmap("../res/images/heightmap_256.ppm", 1, WORLD_SIZE);

	/* Iterate over all points in the heightmap */
	for(x = 0; x < world->width; x++) {
		for(z = 0; z < world->height; z++) {
			Vertex *vtx;
			ColorRGB *col;

			idx = twodim(z, x, w);

			vtx = &vertices[idx];
			col = &colors[idx];

			/* Set height */
			tmp = heightmap[idx] = 40 * heightmapImage[x][z] + 10;

			/* Set the position of the vertex */
			vtx->x = x - 128.0;
			vtx->y = tmp;
			vtx->z = z - 128.0;

			if (tmp <= 15) {
				col->r = 0.79;
				col->g = 0.69;
				col->b = 0.39;	
			}
			else if (tmp > 15 && tmp <= 20) {
				col->r = 0.53;
				col->g = 0.72;
				col->b = 0.32;
			}
			else if (tmp > 20 && tmp <= 30) {
				col->r = 0.31;
				col->g = 0.67;
				col->b = 0.36;
			}
			else if (tmp > 30 && tmp <= 35) {
				col->r = 0.47;
				col->g = 0.47;
				col->b = 0.47;
			}
			else {	
				col->r = 0.78;
				col->g = 0.78;
				col->b = 0.82;
			}
		}
	}

	indices = genIdxBuf(world->width, &indlen);
	if(indices == NULL) {
		printf("Failed to get indices.\n");
		exit(1);
	}
	world->indlen = indlen;

	/* Start creating the model for the terrain */
	mdl = mdlBegin();
	if(mdl == NULL) {
		printf("Failed to create model.\n");
		exit(1);
	}

	mdl->shader = shdBegin(); 
	if(mdl->shader == NULL) {
		printf("Failed to create shader.\n");
		exit(1);
	}


	shdAttachVtx(mdl->shader, "../res/shaders/terrain.vert");
	shdAttachFrg(mdl->shader, "../res/shaders/terrain.frag");

	mdlLoadVtx(mdl, vertices, vsz, indices, indlen);
	mdlAttachColBuf(mdl, colors, vsz);

	glBindAttribLocation(mdl->shader->prog, 0, "vtxPos");
	glBindAttribLocation(mdl->shader->prog, 1, "vtxCol");

	/* Finish creating the model for the terrain */
	if(mdlEnd(mdl) != 0) {
		exit(1);
	}
	world->model = mdl;

	return(0);
}

/*
 * Draw a world using OpenGL.
 *
 * @world: A pointer to the world to draw
 */
void renderTerrain(World *world) 
{
	mdlRender(world->model);	
}

void calculateFaceNormals(World *world)
{    
	int x, z, w, idx;
	float *heightmap;
	float ***faceNormals;

	heightmap = world->heightmap;
	w = world->width;


	/* Calculate normals */
	for(x = 0; x < world->width; x++) {
		for(z = 0; z < world->height; z++) {
			float vx, vy, vz, len;
			float t1[3], t2[3], t3[3];
			float v1[3], v2[3];	

			idx = twodim(z, x, w);

			/* x, z */
			t1[0] = x; 
			t1[1] = heightmap[idx]; 
			t1[2] = z;

			/* x + 1, z */
			t3[0] = x+1; 
			t3[1] = heightmap[idx + w]; 
			t3[2] = z;

			/* x, z + 1 */
			t2[0] = x; 
			t2[1] = heightmap[idx + 1]; 
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
	float px, pz, h;
	int w, idx;
	float *heightmap;

	/* Coordinate (0,0) corresponds with middle of terrain map */
	float ix = x + (world->width / 2.0);
	float iz = z + (world->height / 2.0);

	heightmap = world->heightmap;
	w = world->width;

	/* If outside of terrain, set height to 0 */
	if(ix < 0 || ix >= world->width - 1) {
		return(3);
	}
	if(iz < 0 || iz >= world->height - 1) {
		return(3);
	}

	idx = twodim(floor(iz), floor(ix), w);
	if(heightmap[idx] < 1) {
		return(1);
	}

	/* B(0,1) ------ C(1,1) */
	/*   |      pos    |    */
	/* A(0,0) ------ D(1,0) */
	idx = twodim(floor(iz), floor(ix), w);
	A = heightmap[idx];
	idx = twodim(floor(iz) + 1, floor(ix), w);
	B = heightmap[idx];
	idx = twodim(floor(iz) + 1, floor(ix) + 1, w);
	C = heightmap[idx];
	idx = twodim(floor(iz), floor(ix) + 1, w);
	D = heightmap[idx];

	/* Calculate percent position is along x and z, in [0,1] */
	px = ix - floor(ix);
	pz = iz - floor(iz);

	/* Calculate height, interpolating between 4 points surrounding current pos */
	/* This formula is from wikipedia.com/Bilinear_interpolation */
	h = (A * (1 - px) * (1 - pz)) + (D * (px) * (1 - pz));
	h += (B * (1 - px) * (pz)) + (C * (px) * (pz));

	return (h);
}

float *getNormal(World *world, float x, float z) 
{
	int ix, iz;
	float ***faceNormals;


	/* Coordinate (0,0) corresponds with middle of terrain map */
	ix = x + (world->width / 2.0);
	iz = z + (world->height / 2.0);

	/* If outside of terrain, return null */
	if(ix < 0 || ix >= world->width - 1) {
		return(NULL);
	}
	if(iz < 0 || iz >= world->height - 1) {
		return(NULL);
	}

	return(faceNormals[ix][iz]);
}
