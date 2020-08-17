#include "world.h"
#include "mbasic.h"
#include "world_utils.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Redefine the global world-wrapper */
struct world_wrapper world;


static int twodim(int x, int y, int w) {return y * w + x;}


extern int wld_init(void)
{
	vec2_t del;

	world.size[0] = CHUNK_SIZE;
	world.size[1] = CHUNK_SIZE;

	del[0] = world.size[0] / 2.0;
	del[1] = world.size[1] / 2.0;	

	world.min_pos[0] = -del[0];
	world.min_pos[1] = -del[1];

	world.max_pos[0] = del[0];
	world.max_pos[1] = del[1];

	world.ptnum = (CHUNK_SIZE * CHUNK_SIZE);
	if(!(world.heights = malloc(sizeof(float) * (world.ptnum))))
		return -1;

	memset(world.heights, 0, world.ptnum);
	memset(&world.rot, 0, sizeof(vec3_t));

	if(wld_gen_terrain() < 0) {
		ERR_LOG(("Failed to generate terrain"));	
		free(world.heights);
		return -1;
	}

	return 0;
}


extern void wld_close(void)
{
	if(!world.heights)
		return;

	free(world.heights);
}


extern void wld_render(float interp) 
{
	mat4_t idt;

	if(interp) {/* Prevent warning for not using parameter */}

	mat4_idt(idt);
	mdl_render(world.terrain, idt);
}


extern float wld_get_height(float x, float z)
{
	float ret = 0.0;
	vec2_t rel_pos, coord;
	int2_t map_idx;
	float heights[4];

	rel_pos[0] = ABS(world.min_pos[0] - x);
	rel_pos[1] = ABS(world.min_pos[1] - z);

	map_idx[0] = floor(rel_pos[0]);
	map_idx[1] = floor(rel_pos[1]);

	/* Check if the position is in range */
	if(map_idx[0] < 0 || map_idx[0] >= world.size[0] ||
			map_idx[1] < 0 || map_idx[1] >= world.size[1])
		return 0;

	heights[0] = world.heights[twodim(map_idx[0], map_idx[1], world.size[0])];
	heights[1] = world.heights[twodim(map_idx[0] + 1, map_idx[1], world.size[0])];
	heights[2] = world.heights[twodim(map_idx[0], map_idx[1] + 1, world.size[0])];
	heights[3] = world.heights[twodim(map_idx[0] + 1, map_idx[1] + 1, world.size[0])];

	coord[0] = rel_pos[0] - floor(rel_pos[0]);
	coord[1] = rel_pos[1] - floor(rel_pos[1]);

	if(coord[0] <= (1 - coord[1])) {
		vec3_t v1, v2, v3;

		v1[0] = 0.0;
		v1[1] = heights[0];
		v1[2] = 0.0;

		v2[0] = 1.0;
		v2[1] = heights[1];
		v2[2] = 0.0;

		v3[0] = 0.0;
		v3[1] = heights[2];
		v3[2] = 1.0;

		ret = vec3_barry_centric(v1, v2, v3, coord);
	}
	else {
		vec3_t v1, v2, v3;

		v1[0] = 1.0;
		v1[1] = heights[1];
		v1[2] = 0.0;

		v2[0] = 1.0;
		v2[1] = heights[3];
		v2[2] = 1.0;

		v3[0] = 0.0;
		v3[1] = heights[2];
		v3[2] = 1.0;

		ret = vec3_barry_centric(v1, v2, v3, coord);
	}

	return ret;
}


extern int wld_gen_terrain(void)
{
	int x;
	int z;
	int i;
	float *heights = NULL;

	world.size[0] = CHUNK_SIZE;
	world.size[1] = CHUNK_SIZE;

	heights = world.heights;
	for(x = 0; x < world.size[0]; x++) {
		for(z = 0; z < world.size[1]; z++) {
			i = twodim(z, x, world.size[1]);
			heights[i] = 0;
		}
	}

	/* Allocate memory for the model-struct */
	if((world.terrain = mdl_get("wld")) < 0)
		return -1;

	return 0;
}
