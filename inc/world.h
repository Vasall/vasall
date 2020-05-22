#ifndef _WORLD_H
#define _WORLD_H

#include "asset.h"
#include "model.h"

#define WORLD_SIZE 256

struct world {
	int2_t size;
	int ptnum;
	vec3_t pos;
	vec2_t min_pos, max_pos;
	float *heights;
	vec3_t rot;
	short terrain;
};


extern struct world world;


extern int wld_init(void);
extern void wld_close(void);

extern void wld_render(void);

extern float wld_get_height(float x, float z);

extern int wld_gen_terrain(void);

#endif
