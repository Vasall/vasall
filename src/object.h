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
#define OBJ_M_DATA 0x01

struct object_table {
	uint32_t mask[OBJ_SLOTS];
	vec3_t pos[OBJ_SLOTS];
	vec3_t vel[OBJ_SLOTS];
	vec3_t dir[OBJ_SLOTS];
	mat4_t mat[OBJ_SLOTS];
	short model[OBJ_SLOTS];
	short anim[OBJ_SLOTS];
	short prog[OBJECTS_SLOTS];
	int len[OBJ_SLOTS];
	char data[OBJ_SLOTS][OBJ_DATA_MAX];
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
 * Update an object and use pyhsical calculations to adjust the attributes of 
 * the object.
 *
 * @obj: Pointer to the object to update
 * @delt: The time since the last frame
 */
void obj_sys_update(struct object *obj, float delt);

/* 
 * Render the model of an object on the screen.
 *
 * obj: Pointer to the object to render
*/
void obj_sys_render(struct object *obj);

/*
 * Update the model-matrix of a given model.
 *
 * @obj: The model to update the mode-matrix for
 */
void obj_update_mat(struct object *obj);

/* 
 * Output info about the object in the terminal
 *
 * @obj: Pointer to the object to display data about
*/
void obj_print(struct object *obj);

#endif
