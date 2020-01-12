#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include "world.h"
#include "global.h"
#include "vec.h"
#include "stdutil.h"
#include "shader.h"
#include "loader.h"
#include "wld_idx_gen.h"

float delF = 0.0;

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
	Vertex *vertices, *lastRow;
	ColorRGB *colors;
	Vec3 *normals;
	uint32_t *indices;
	int indlen;
	Model *mdl;

	w = world->xsize;

	/* Calculate the amount of vertices */
	vtxnum = calculateVertexCount(w);

	/* Initialize the vertex-array */
	vertices = malloc(vtxnum * sizeof(Vertex));
	if(vertices == NULL) {
		printf("Failed to allocate space for vertices.\n");
		return(-1);
	}


	/* Create an array for all vertice-colors */
	colors = malloc(vtxnum * sizeof(ColorRGB));
	if(colors == NULL) {
		printf("Failed to allocate space for colors.\n");
		return(-1);
	}

	/* Create an array for all normal-vectors */
	normals = malloc(vtxnum * sizeof(Vec3));
	if(normals == NULL) {
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

	lastRow = malloc((world->xsize - 1) * sizeof(Vertex) * 2);

	/* Iterate over all points in the heightmap */
	idx = 0;
	for(z = 0; z < world->zsize - 1; z++) {
		for(x = 0; x < world->xsize - 1; x++) {
			Vertex ctl, ctr, cbl, cbr;
			
			xpos = x - 128.0;
			zpos = z - 128.0;

			ctl.x = xpos;
			ctl.y = 40.0 * hImg[x][z] + 10.0;
		       	ctl.z = zpos;

			ctr.x = xpos + 1.0;
		        ctr.y = 40.0 * hImg[x + 1][z] + 10.0;
			ctr.z = zpos;


			cbl.x = xpos;
		        cbl.y = 40.0 * hImg[x][z + 1] + 10.0;
			cbl.z = zpos + 1.0;

			cbr.x = xpos + 1.0;
			cbr.y = 40.0 * hImg[x + 1][z + 1] + 10.0;
			cbr.z = zpos + 1.0;

			vertices[idx] = ctl;
			idx++;

			if(z != world->zsize - 2 || x == world->xsize - 2) {	
				vertices[idx] = ctr;
				idx++;
			}

			if(z == world->zsize - 2) {
				lastRow[x * 2] = cbl;
				lastRow[x * 2 + 1] = cbr;
			}
		}
	}

	for(j = 0; j < world->xsize  - 1; j++) {
		Vertex left = lastRow[j * 2];
		Vertex right = lastRow[j * 2 + 1];

		if(left.x == 0 || right.x == 1) {
			vertices[idx] = left;
			idx++;
		}

		vertices[idx] = right;
		idx++;
	}

	indices = generateIndexBuffer(world->xsize, &indlen);
	if(indices == NULL) {
		printf("Failed to get indices.\n");
		return(-1);
	}

	/* Calculate the colors for the vertices */
	for(j = 0; j < indlen - 2; j += 3) {
		ColorRGB col;
		Vertex v, mid = {0.0, 0.0, 0.0};

		for(i = 0; i < 3; i++) {
			v = vertices[indices[j + i]];
			vecAdd(&mid, v);
		}
		vecInvScl(&mid, 3);

		if (mid.y <= 15) {
			col.r = 0.79;
			col.g = 0.69;
			col.b = 0.39;	
		}
		else if (mid.y > 15 && mid.y <= 20) {
			col.r = 0.53;
			col.g = 0.72;
			col.b = 0.32;
		}
		else if (mid.y > 20 && mid.y <= 30) {
			col.r = 0.31;
			col.g = 0.67;
			col.b = 0.36;
		}
		else if (mid.y > 30 && mid.y <= 35) {
			col.r = 0.47;
			col.g = 0.47;
			col.b = 0.47;
		}
		else {	
			col.r = 0.78;
			col.g = 0.78;
			col.b = 0.82;
		}

		colors[indices[j]] = col;
	}

	/* Calculate normal-vectors for all triangles */
	for(j = 0; j < indlen - 2; j += 3) {
		Vec3 v1 = vertices[indices[j]];
		Vec3 v2 = vertices[indices[j + 1]];
		Vec3 v3 = vertices[indices[j + 2]];
		Vec3 del1 = vecSubRet(v2, v1);
		Vec3 del2 = vecSubRet(v2, v3);
		Vec3 nrm = vecNrmRet(vecCross(del1, del2));
		
		normals[indices[j]] = nrm;
	}

	/* Start creating the model for the terrain */
	mdl = mdlBegin();
	if(mdl == NULL) {
		printf("Failed to create model.\n");
		return(-1);
	}

	mdlLoadVtx(mdl, vertices, vtxnum, indices, indlen);
	mdlAttachColBuf(mdl, colors, vtxnum);
	mdlAttachBuf(mdl, normals, sizeof(Vec3), vtxnum, 1);

	shdAttachVtx(mdl->shader, "../res/shaders/terrain.vert");
	shdAttachFrg(mdl->shader, "../res/shaders/terrain.frag");

	glBindAttribLocation(mdl->shader->prog, 0, "vtxPos");
	glBindAttribLocation(mdl->shader->prog, 1, "vtxCol");
	glBindAttribLocation(mdl->shader->prog, 2, "vtxNrm");

	/* Finish creating the model for the terrain */
	if(mdlEnd(mdl) != 0) {
		return(-1);
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
