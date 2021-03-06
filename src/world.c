#include "world.h"

#include "extmath.h"
#include "world_utils.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Redefine the global world-wrapper */
struct wld_wrapper g_wld;

#if 0
static int twodim(int x, int y, int w) {return y * w + x;}
#endif

extern int wld_init(void)
{
	int i;
	float tmp;

	/* Set the position and size of the world */
	tmp = _CHUNK_SIZE / 2.0; 
	vec2_set(g_wld.pos, -tmp, -tmp);
	vec2_set(g_wld.size, _CHUNK_SIZE, _CHUNK_SIZE);


	/* Initialize the collision-boxes */
	for(i = 0; i < WLD_CHUNK_OBJ_LIM; i++) {
		g_wld.col_box[i].obj = -1;	
	}

	/* Initialize the collision-min-max-points */
	for(i = 0; i < 2 * WLD_CHUNK_OBJ_LIM; i++) {
		g_wld.col_point[0][i].data = 0;
		g_wld.col_point[1][i].data = 0;
		g_wld.col_point[2][i].data = 0;
	}

	g_wld.skybox = -1;

	return 0;
}


extern void wld_close(void)
{

}


extern void wld_render(float interp) 
{
	if(interp) {}
	if(g_wld.skybox)
		mdl_render(g_wld.skybox, NULL, NULL, NULL);
	return;
}


extern void wld_set_skybox(short mdl)
{
	g_wld.skybox = mdl;
}


#if 0
static short wld_col_pnt_add(char axis, int flg, short box, float value)
{
	int i;

	for(i = 0; i < 2 * WLD_CHUNK_OBJ_LIM; i++) {
		if(g_wld.col_point[(int)axis][i].data == 0) {
			g_wld.col_point[(int)axis][i].data = (box + 1) | flg;
			g_wld.col_point[(int)axis][i].value = value;

			return i; 
		}
	}

	return -1;
}

extern int wld_col_add(short obj, vec3_t pos, cube_t colbox, short *idx)
{
	short i;
	short j;
	short slot;
	short tmp;
	vec3_t relpos;
	vec3_t min;
	vec3_t max;


	/* Get a free slot in the array */
	slot = -1;
	for(i = 0; i < WLD_CHUNK_OBJ_LIM; i++) {
		if(g_wld.col_box[i].obj == -1) {
			slot = i;
			break;
		}
	}

	/* If no slots are available */
	if(slot < 0)
		return -1;

	/* Set the index of the object */
	g_wld.col_box[slot].obj = obj;

	/* Copy the cube-struct */
	g_wld.col_box[slot].box = colbox;

	/* Clear the point indices */
	for(i = 0; i < 3; i++) {
		g_wld.col_box[slot].min[i] = -1;
		g_wld.col_box[slot].max[i] = -1;
	}

	/* Clear the list of colliding objects */
	for(i = 0; i < 5; i++)
		g_wld.col_box[slot].colobj[i] = -1;

	/* Add the endpoints to the point-arrays */
	vec3_add(colbox.pos, pos, relpos);

	vec3_sub(relpos, colbox.scl, min);
	vec3_add(relpos, colbox.scl, max);


	/* Insert the endpoints into the corresponding arrays */
	for(i = 0; i < 3; i++) {
		/*
		 * Insert the min-endpoint.
		 */

		/* Find a slot */
		tmp = -1;
		for(j = 0; j < WLD_CHUNK_OBJ_LIM * 2; j++) {
			if(g_wld.col_point[i][j].data == 0) {
				tmp = j;
			}
		}

		if(tmp < 0)
			goto err_remv_points;

		/* Add point */
		g_wld.col_point[i][j].data = slot | WLD_COL_MIN; 
		g_wld.col_point[i][j].value = min[i];

		/* Save index of point */
		g_wld.col_box[slot].min[i] = tmp;	

		/*
		 * Insert the max-endpoint.
		 */

		/* Find slot */
		tmp = -1;
		for(j = 0; j < WLD_CHUNK_OBJ_LIM * 2; j++) {
			if(g_wld.col_point[i][j].data == 0) {
				tmp = j;
			}
		}

		if(tmp < 0)
			goto err_remv_points;

		/* Add point */
		g_wld.col_point[i][j].data = slot | WLD_COL_MAX; 
		g_wld.col_point[i][j].value = min[i];

		/* Save index of point */
		g_wld.col_box[slot].max[i] = tmp;	
	}

	return 0;

err_remv_points:
	/*
	 * Clear the endpoints
	 */
	for(i = 0; i < 3; i++) {
		if((tmp = g_wld.col_box[slot].min[i]) >= 0)
			g_wld.col_point[i][tmp].data = 0;

		if((tmp = g_wld.col_box[slot].max[i]) >= 0)
			g_wld.col_point[i][tmp].data = 0;
	}

	/*
	 * Clear the collision-box
	 */
	g_wld.col_box[slot].obj = -1;

	return -1;
}


extern int wld_col_move(short slot, vec3_t pos)
{
	int i;
	vec3_t min;
	vec3_t max;
	vec3_t relpos;

	/* Check if slot is valid */
	if(slot < 0 || g_wld.col_box[slot].obj < 0)
		return -1;

	/* Calculate new endpoints */
	vec3_add(g_wld.col_box[slot].box.pos, pos, relpos);

	vec3_sub(relpos, g_wld.col_box[slot].box.scl, min);
	vec3_add(relpos, g_wld.col_box[slot].box.scl, max);


	/* Update points */
	for(i = 0; i < 3; i++) {
		g_wld.col_point[i][g_wld.col_box[slot].min[i]].value = min[i];
		g_wld.col_point[i][g_wld.col_box[slot].max[i]].value = max[i];
	}

	return 0;
}


extern int wld_col_update(void)
{
	int i;
	int j;

	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {

		}
	}

	return 0;
}
#endif
