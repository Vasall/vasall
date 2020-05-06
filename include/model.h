#ifndef _MODEL_H
#define _MODEL_H

#include "vec.h"
#include "mat.h"
#include "sdl.h"
#include "asset.h"
#include "camera.h"

#define MDL_NAME_MAX            8
#define MDL_SLOTS             256

enum mdl_status {
	MDL_OK =                0,
	MDL_ERR_CREATING =      1,
	MDL_ERR_MESH =          2,
	MDL_ERR_TEXTURE =       3,
	MDL_ERR_LOADING =       4,
	MDL_ERR_SHADER =        5,
	MDL_ERR_FINISHING =     6
};

struct model {
	char name[9];
	uint32_t vao;
	uint32_t idx_bao;
	int idx_num;
	int *idx_buf;
	uint32_t vtx_bao;
	int vtx_num;
	float *vtx_buf;
	short tex;
	short shd;
	uint8_t status;
};


extern struct model *models[MDL_SLOTS];


extern int mdl_init(void);
extern void mdl_close(void);

extern short mdl_set(char *name);
extern short mdl_get(char *name);
extern void mdl_del(short slot);

extern void mdl_set_mesh(short slot, int idxnum, int *idx, int vtxnum, 
		vec3_t *vtx, vec3_t *nrm, void *col, uint8_t col_flg);
extern void mdl_set_texture(short slot, short tex);
extern void mdl_set_shader(short slot, short shd);

extern short mdl_load(char *name, char *amo, short tex, short shd);

extern void mdl_render(short slot, mat4_t mat);

#endif
