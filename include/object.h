#ifndef _OBJECT_H
#define _OBJECT_H

#include "vec.h"
#include "mat.h"
#include "mbasic.h"
#include "sdl.h"
#include "asset.h"
#include "model.h"

#define OBJ_SLOTS 128
#define OBJ_DATA_MAX 128

#define OBJ_M_NONE 0x00
#define OBJ_M_MOVE 0x01
#define OBJ_M_MODEL 0x02
#define OBJ_M_DATA 0x03

#define OBJ_M_ENTITY (OBJ_M_MOVE|OBJ_M_MODEL)

enum object_attr {
	OBJ_ATTR_MASK =   0x01,
	OBJ_ATTR_POS =    0x02,
	OBJ_ATTR_VEL =    0x03,
	OBJ_ATTR_DIR =    0x04,
	OBJ_ATTR_BUF =    0x05

};

struct object_table {
	uint32_t  mask[OBJ_SLOTS];
	vec3_t    pos[OBJ_SLOTS];
	vec3_t    vel[OBJ_SLOTS];
	vec3_t    dir[OBJ_SLOTS];
	short     model[OBJ_SLOTS];
	short     anim[OBJ_SLOTS];
	float     prog[OBJ_SLOTS];
	mat4_t    mat[OBJ_SLOTS];
	int       len[OBJ_SLOTS];
	char      buf[OBJ_SLOTS][OBJ_DATA_MAX];
};


extern struct object_table objects;


extern int obj_init(void);
extern void obj_close(void);

extern short obj_set(uint32_t id, uint32_t mask, vec3_t pos, short model,
		char *data, int len);
extern void obj_del(short slot);

extern int obj_mod(short slot, short attr, void *data, int len);

extern short obj_sel_id(uint32_t id);

extern void obj_update_matrix(short slot);

extern void obj_print(short slot);

extern void obj_sys_update(float delt);
extern void obj_sys_render(void);

#endif
