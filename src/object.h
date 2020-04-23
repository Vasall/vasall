#ifndef _OBJECT_H
#define _OBJECT_H

#include "vec.h"
#include "mat.h"
#include "list.h"
#include "model.h"

#include <stdint.h>

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

/*
 * Initialize a new object-table and reset all entries.
 *
 * Returns: Either a pointer to the created object-table or NULL if an error
 * 	occurred
 */
struct object_table *obj_init(void);

/*
 * Close an old object-table and free the allocated memory.
 *
 * @tbl: Pointer to the object-table
 */
void obj_close(struct object_table *tbl);

/*
 * Create and initialize a new object and then insert it into the object-table
 *
 * @tbl: Pointer to the object-table
 * @mask: The mask to use for the object
 * @pos: The initial position of the object
 * @model: A slot in the model-table 
 * @data: Additional data for the object
 * @len: The length of the data-buffer in bytes
 *
 * Returns: Either the slot of the object in the object-table or -1 if an error
 * 	occurred
 */
short obj_set(struct object_table *tbl, uint32_t mask, vec3_t pos, short model,
		char *data, int len);

/*
 * Delete an object and remove it from the object-table.
 *
 * @tbl: Pointer to the object-table
 * @slot: The slot of the object to delete
 */
void obj_del(struct object_table *tbl, short slot);

/*
 * Modify an attribute of an object.
 *
 * @tbl: Pointer to the object-table
 * @slot: The slot of the object in the object-table
 * @attr: THE attribute to modify
 * @data: Pointer to the data
 * @len: The length of the data-buffer in bytes
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
int obj_mod(struct object_table *tbl, short slot, short attr, 
		void *data, int len);

/*
 * Update the model-matrix of a given model.
 *
 * @tbl: Pointer to the object-table
 * @slot: The slot in the object-table
 */
void obj_update_matrix(struct object_table *tbl, short slot);

/* 
 * Output info about the object in the terminal.
 *
 * @tbl: Pointer to the object-table
 * @slot: The slot of the object in the object-table
 */
void obj_print(struct object_table *tbl, short slot);

/*
 * Update all objects in an object-table according to their mask.
 *
 * @tbl: Pointer to the object-table
 * @delt: The time since the last frame
 */
void obj_sys_update(struct object_table *tbl, float delt);

/* 
 * Render all renderable objects in an object-table.
 *
 * @tbl: Pointer to the object table
 */
void obj_sys_render(struct object_table *tbl);

#endif
