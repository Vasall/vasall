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

/* Redefine external variables */
struct world *world = NULL;

int twodim(int x, int y, int w) {return(y * w + x);}

/*
 * Initialize the global world-struct
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int wldCreate(void)
{
	/* Allocate space for the world-struct */
	world = (struct world *)malloc(sizeof(struct world));
	if(world == NULL) return(-1);

	/* Allocate space for the heightmap */
	world->xsize = WORLD_SIZE;
	world->zsize = WORLD_SIZE;
	world->ptnum = (WORLD_SIZE * WORLD_SIZE);
	world->heights = malloc(sizeof(float) * (world->ptnum));
	if(world->heights == NULL) return(-1);

	memset(world->heights, 0, world->ptnum);

	memset(&world->pos, 0, sizeof(Vec3));
	memset(&world->rot, 0, sizeof(Vec3));

	/* Set the terrain-model-pointer to NULL */
	world->terrain = NULL;

	/* Generate the terrain */
	wldGenTerrain();

	return(0);
}

/*
 * Delete a world and free allocated memory.
 *
 * @world: Pointer to the world
 * 	to delete
 */
void wldDestroy(void)
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
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int wldGenTerrain(void)
{
	int vtxnum, x, z, w, count, i, j, idxnum, *idx = NULL;
	float **hImg = NULL, *heights = NULL, xpos, zpos;
	Vec3 *vtx = NULL, *lastRow = NULL, *nrm = NULL, *col = NULL;
	struct model *mdl;

	w = world->xsize;

	/* Calculate the amount of vertices */
	vtxnum = calcVertexNum(w);

	/* Initialize the vertex-array */
	vtx = calloc(vtxnum, VEC3_SIZE);
	if(vtx == NULL) return(-1);

	/* Create an array for all vertice-colors */
	col = calloc(vtxnum, VEC3_SIZE);
	if(col == NULL) return(-1);

	nrm = calloc(vtxnum, VEC3_SIZE);

	hImg = loadPPMHeightmap("../res/images/heightmap_256.ppm", 1, 256);
	heights = world->heights;
	for(x = 0; x < world->xsize; x++) {
		for(z = 0; z < world->zsize; z++) {
			i = twodim(z, x, world->zsize);
			heights[i] = (40.0 * hImg[x][z] + 10.0) * 1.0;
		}
	}
	lastRow = malloc((world->xsize - 1) * VEC3_SIZE * 2);
	if(lastRow == NULL) return(-1);

	/* Iterate over all points in the heightmap */
	count = 0;
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

			vecCpy(vtx[count], ctl);
			count++;

			if(z != world->zsize - 2 || x == world->xsize - 2) {	
				vecCpy(vtx[count], ctr);
				count++;
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
			vecCpy(vtx[count], left);
			count++;
		}

		vecCpy(vtx[count], right);
		count++;
	}

	idx = (int *)genIndexBuf(world->xsize, &idxnum);
	if(idx == NULL) return(-1);

	/* Calculate the colors for the vertices */
	for(j = 0; j < idxnum - 2; j += 3) {
		Vec3 col_tmp;
		Vec3 v, mid;
		vecSet(mid, 0.0, 0.0, 0.0);

		for(i = 0; i < 3; i++) {
			vecCpy(v, vtx[idx[j + i]]);
			vecAdd(mid, v, mid);
		}
		vecInvScl(mid, 3, mid);

		if (mid[1] <= 15) {
			col_tmp[0] = 0.79;
			col_tmp[1] = 0.69;
			col_tmp[2] = 0.39;	
		}
		else if (mid[1] > 15 && mid[1] <= 20) {
			col_tmp[0] = 0.53;
			col_tmp[1] = 0.72;
			col_tmp[2] = 0.32;
		}
		else if (mid[1] > 20 && mid[1] <= 30) {
			col_tmp[0] = 0.31;
			col_tmp[1] = 0.67;
			col_tmp[2] = 0.36;
		}
		else if (mid[1] > 30 && mid[1] <= 35) {
			col_tmp[0] = 0.47;
			col_tmp[1] = 0.47;
			col_tmp[2] = 0.47;
		}
		else {	
			col_tmp[0] = 0.78;
			col_tmp[1] = 0.78;
			col_tmp[2] = 0.82;
		}

		memcpy(col[idx[j]], col_tmp, VEC3_SIZE);
	}

	/* Allocate memory for the model-struct */
	if((mdl = mdlCreate("wld_")) == NULL) goto failed;
	mdlSetShader(mdl, "wld_");
	mdlSetMesh(mdl, idxnum, (int32_t *)idx, vtxnum, vtx, nrm, col, 0);
	if(mdlFinish(mdl) < 0) goto failed;

	world->terrain = mdlGet("wld_");
	if(world->terrain == NULL) goto failed;

	return(0);

failed:
	return(-1);
}

/*
 * Draw a world using OpenGL.
 *
 * @world: A pointer to the world to draw
 */
void wldRender(void) 
{
	Mat4 idt;
	mat4Idt(idt);
	mdlRender(world->terrain, idt);
}

float wldGetHeight(float x, float z)
{
	if(x || z) return(0);
	return(0);
}
