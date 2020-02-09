#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include "world.h"
#include <math.h>
#include "global.h"
#include "vec.h"
#include "utils.h"
#include "shader.h"
#include "loader.h"
#include "world_utils.h"

/* Redefine external variables */
struct world *world = NULL;

static int twodim(int x, int y, int w) {return(y * w + x);}
static float _abs(float val) {return(val < 0) ? (-val) : (val);}

/*
 * Initialize the global world-struct
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int wldCreate(void)
{
	Vec2 del;

	/* Allocate space for the world-struct */
	world = (struct world *)malloc(sizeof(struct world));
	if(world == NULL) return(-1);

	/* Allocate space for the heightmap */
	world->size[0] = WORLD_SIZE;
	world->size[1] = WORLD_SIZE;

	del[0] = world->size[0] / 2.0;
	del[1] = world->size[1] / 2.0;	

	memset(world->pos, 0, VEC3_SIZE);

	world->min_pos[0] = world->pos[0] - del[0];
	world->min_pos[1] = world->pos[2] - del[1];
	
	world->max_pos[0] = world->pos[0] + del[0];
	world->max_pos[1] = world->pos[2] + del[1];
	
	world->ptnum = (WORLD_SIZE * WORLD_SIZE);
	world->heights = malloc(sizeof(float) * (world->ptnum));
	if(world->heights == NULL) return(-1);

	memset(world->heights, 0, world->ptnum);

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

/*
 * Get the height of the world at a
 * given position.
 *
 * @x: The x-position in the world
 * @z: The z-position in the world
 *
 * Returns: The height at the given position
 */
float wldGetHeight(float x, float z)
{
	float ret = 0.0;

	Vec2 rel_pos, coord;
	Int2 map_idx;
	float heights[4];

	rel_pos[0] = _abs(world->min_pos[0] - x);
	rel_pos[1] = _abs(world->min_pos[1] - z);

	map_idx[0] = floor(rel_pos[0]);
	map_idx[1] = floor(rel_pos[1]);

	/* Check if the position is in range */
	if(map_idx[0] < 0 || map_idx[0] >= world->size[0] ||
				map_idx[1] < 0 || map_idx[1] >= world->size[1]) {
		return(0);
	}

	heights[0] = world->heights[twodim(map_idx[0], 
			map_idx[1], world->size[0])];
	heights[1] = world->heights[twodim(map_idx[0] + 1, 
			map_idx[1], world->size[0])];
	heights[2] = world->heights[twodim(map_idx[0], 
			map_idx[1] + 1, world->size[0])];
	heights[3] = world->heights[twodim(map_idx[0] + 1, 
			map_idx[1] + 1, world->size[0])];

	coord[0] = rel_pos[0] - floor(rel_pos[0]);
	coord[1] = rel_pos[1] - floor(rel_pos[1]);

	if(coord[0] <= (1 - coord[1])) {
		Vec3 v1, v2, v3;
	       	
		v1[0] = 0.0;
		v1[1] = heights[0];
		v1[2] = 0.0;

		v2[0] = 1.0;
		v2[1] = heights[1];
		v2[2] = 0.0;

		v3[0] = 0.0;
		v3[1] = heights[2];
		v3[2] = 1.0;

		ret = vecBarryCentric(v1, v2, v3, coord);
	}
	else {
		Vec3 v1, v2, v3;
	       	
		v1[0] = 1.0;
		v1[1] = heights[1];
		v1[2] = 0.0;

		v2[0] = 1.0;
		v2[1] = heights[3];
		v2[2] = 1.0;

		v3[0] = 0.0;
		v3[1] = heights[2];
		v3[2] = 1.0;
		
		ret = vecBarryCentric(v1, v2, v3, coord);

	}


	return(ret);
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

	w = (int)world->size[0];

	/* Calculate the amount of vertices */
	vtxnum = calcVertexNum(w);

	/* Initialize the vertex-array */
	vtx = calloc(vtxnum, VEC3_SIZE);
	if(vtx == NULL) return(-1);

	/* Create an array for all vertice-colors */
	col = calloc(vtxnum, VEC3_SIZE);
	if(col == NULL) return(-1);

	nrm = calloc(vtxnum, VEC3_SIZE);
	if(nrm == NULL) return(-1);

	hImg = loadPPMHeightmap("../res/images/heightmap_256.ppm", 1, 256);
	heights = world->heights;
	for(x = 0; x < world->size[0]; x++) {
		for(z = 0; z < world->size[1]; z++) {
			i = twodim(z, x, world->size[1]);
			heights[i] = (40.0 * hImg[x][z] + 10.0) * 1.0;
		}
	}
	lastRow = malloc(((int)world->size[0] - 1) * VEC3_SIZE * 2);
	if(lastRow == NULL) return(-1);

	/* Iterate over all points in the heightmap */
	count = 0;
	for(z = 0; z < world->size[1] - 1; z++) {
		for(x = 0; x < world->size[0] - 1; x++) {
			Vec3 ctl, ctr, cbl, cbr;
			
			xpos = x - 128.0;
			zpos = z - 128.0;

			ctl[0] = xpos;
			ctl[1] = heights[twodim(x, z, world->size[0])];
		       	ctl[2] = zpos;

			ctr[0] = xpos + 1.0;
			ctr[1] = heights[twodim(x + 1, z, world->size[0])];
			ctr[2] = zpos;

			cbl[0] = xpos;
			cbl[1] = heights[twodim(x, z + 1, world->size[0])];
			cbl[2] = zpos + 1.0;

			cbr[0] = xpos + 1.0;
			cbr[1] = heights[twodim(x + 1, z + 1, world->size[0])];
			cbr[2] = zpos + 1.0;

			vecCpy(vtx[count], ctl);
			count++;

			if(z != world->size[1] - 2 || x == world->size[0] - 2) {	
				vecCpy(vtx[count], ctr);
				count++;
			}

			if(z == world->size[1] - 2) {
				vecCpy(lastRow[x * 2], cbl);
				vecCpy(lastRow[x * 2 + 1], cbr);
			}
		}
	}

	for(j = 0; j < world->size[0] - 1; j++) {
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

	idx = (int *)genIndexBuf(world->size[0], &idxnum);
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

	/* Calculate the normal-vectors */
	for(i = 0; i < idxnum - 2; i += 3) {
		Vec3 v1, v2, v3, del1, del2, nrm_tmp;
		
		vecCpy(v1, vtx[idx[i]]);
		vecCpy(v2, vtx[idx[i + 1]]);
		vecCpy(v3, vtx[idx[i + 2]]);

		vecSub(v2, v1, del1);
		vecSub(v2, v3, del2);

		vecCross(del1, del2, nrm_tmp);
		vecNrm(nrm_tmp, nrm_tmp);
		vecScl(nrm_tmp, -1, nrm_tmp);

		memcpy(nrm[idx[i]], nrm_tmp, VEC3_SIZE);
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

