#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include "world.h"
#include "global.h"
#include "vec.h"
#include "utils.h"
#include "shader.h"
#include "loader.h"
#include "world_utils.h"

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
	world->xsize = WORLD_SIZE;
	world->zsize = WORLD_SIZE;
	world->ptnum = (WORLD_SIZE * WORLD_SIZE);
	world->heights = (float *)malloc((world->ptnum) * 
			sizeof(float));
	if(world->heights == NULL) {
		return(NULL);
	}

	memset(world->heights, 0, world->ptnum);

	memset(&world->pos, 0, sizeof(Vec3));
	memset(&world->rot, 0, sizeof(Vec3));

	/* Set the terrain-model-pointer to NULL */
	world->terrain = NULL;

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
	free(world->heights);

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
	int vtxnum, x, z, w, idx, i, j;
	float **hImg, *heights, xpos, zpos;
	Vec3 *vertices, *lastRow;
	ColorRGB *colors;
	uint32_t *indices;
	int indlen;
	Model *mdl;

	w = world->xsize;

	/* Calculate the amount of vertices */
	vtxnum = calcVertexNum(w);

	/* Initialize the vertex-array */
	vertices = calloc(vtxnum, VEC3_SIZE);
	if(vertices == NULL) {
		printf("Failed to allocate space for vertices.\n");
		return(-1);
	}


	/* Create an array for all vertice-colors */
	colors = calloc(vtxnum, sizeof(ColorRGB));
	if(colors == NULL) {
		printf("Failed to allocate space for colors.\n");
		return(-1);
	}

	hImg = loadPPMHeightmap("../res/images/heightmap_256.ppm", 1, 256);
	heights = world->heights;
	for(x = 0; x < world->xsize; x++) {
		for(z = 0; z < world->zsize; z++) {
			i = twodim(z, x, world->zsize);
			heights[i] = (40.0 * hImg[x][z] + 10.0) * 0.0;
		}
	}

	lastRow = malloc((world->xsize - 1) * VEC3_SIZE * 2);

	/* Iterate over all points in the heightmap */
	idx = 0;
	for(z = 0; z < world->zsize - 1; z++) {
		for(x = 0; x < world->xsize - 1; x++) {
			Vec3 ctl, ctr, cbl, cbr;
			
			xpos = x - 128.0;
			zpos = z - 128.0;

			ctl[0] = xpos;
			ctl[1] = heights[twodim(x, z, world->xsize)];
		       	ctl[2] = zpos;

			ctr[0] = xpos + 1.0;
			ctr[1] = heights[twodim(x + 1, z, world->xsize)];
			ctr[2] = zpos;


			cbl[0] = xpos;
			cbl[1] = heights[twodim(x, z + 1, world->xsize)];
			cbl[2] = zpos + 1.0;

			cbr[0] = xpos + 1.0;
			cbr[1] = heights[twodim(x + 1, z + 1, world->xsize)];
			cbr[2] = zpos + 1.0;

			vecCpy(vertices[idx], ctl);
			idx++;

			if(z != world->zsize - 2 || x == world->xsize - 2) {	
				vecCpy(vertices[idx], ctr);
				idx++;
			}

			if(z == world->zsize - 2) {
				vecCpy(lastRow[x * 2], cbl);
				vecCpy(lastRow[x * 2 + 1], cbr);
			}
		}
	}

	for(j = 0; j < world->xsize  - 1; j++) {
		Vec3 left, right;
		vecCpy(left, lastRow[j * 2]);
		vecCpy(right, lastRow[j * 2 + 1]);

		if(left[0] == 0 || right[0] == 1) {
			vecCpy(vertices[idx], left);
			idx++;
		}

		vecCpy(vertices[idx], right);
		idx++;
	}

	indices = genIndexBuf(world->xsize, &indlen);
	if(indices == NULL) {
		printf("Failed to get indices.\n");
		return(-1);
	}

	/* Calculate the colors for the vertices */
	for(j = 0; j < indlen - 2; j += 3) {
		ColorRGB col;
		Vec3 v, mid;
		vecSet(mid, 0.0, 0.0, 0.0);

		for(i = 0; i < 3; i++) {
			vecCpy(v, vertices[indices[j + i]]);
			vecAdd(mid, v, mid);
		}
		vecInvScl(mid, 3, mid);

		if (mid[1] <= 15) {
			col.r = 0.79;
			col.g = 0.69;
			col.b = 0.39;	
		}
		else if (mid[1] > 15 && mid[1] <= 20) {
			col.r = 0.53;
			col.g = 0.72;
			col.b = 0.32;
		}
		else if (mid[1] > 20 && mid[1] <= 30) {
			col.r = 0.31;
			col.g = 0.67;
			col.b = 0.36;
		}
		else if (mid[1] > 30 && mid[1] <= 35) {
			col.r = 0.47;
			col.g = 0.47;
			col.b = 0.47;
		}
		else {	
			col.r = 0.78;
			col.g = 0.78;
			col.b = 0.82;
		}

		memcpy(&colors[indices[j]], &col, sizeof(ColorRGB));
	}

	if((mdl = mdlCreate(&world->pos, &world->rot)) == NULL) return(-1);
	mdlSetMesh(mdl, vertices, vtxnum, indices, indlen, 1);
	mdlAddBAO(mdl, colors, sizeof(ColorRGB), vtxnum, 2, 3);
	shdAttachVtx(mdl->shader, "../res/shaders/terrain.vert");
	shdAttachFrg(mdl->shader, "../res/shaders/terrain.frag");
	glBindAttribLocation(mdl->shader->prog, 0, "vtxPos");
	glBindAttribLocation(mdl->shader->prog, 1, "vtxNrm");
	glBindAttribLocation(mdl->shader->prog, 2, "vtxCol");
	if(mdlFinish(mdl) < 0) return(-1);

	world->terrain = mdl;

	return(0);
}

/*
 * Draw a world using OpenGL.
 *
 * @world: A pointer to the world to draw
 */
void wldRender(World *world) 
{
	mdlRender(world->terrain);
}
