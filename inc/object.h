#ifndef _OBJECT_H
#define _OBJECT_H

#include "vec.h"
#include "mat.h"
#include "mbasic.h"
#include "sdl.h"
#include "asset.h"
#include "model.h"

#define OBJ_SLOTS      128
#define OBJ_DATA_MAX   128

#define OBJ_M_NONE     0x00
#define OBJ_M_MOVE     0x01
#define OBJ_M_MODEL    0x02
#define OBJ_M_DATA     0x04
#define OBJ_M_PEER     0x08

#define OBJ_M_ENTITY (OBJ_M_MOVE|OBJ_M_MODEL)
#define OBJ_M_PLAYER (OBJ_M_ENTITY|OBJ_M_PEER)

enum object_attr {
	OBJ_ATTR_ID =     0x00,
	OBJ_ATTR_MASK =   0x01,
	OBJ_ATTR_POS =    0x02,
	OBJ_ATTR_VEL =    0x03,
	OBJ_ATTR_ACL =    0x04,
	OBJ_ATTR_BUF =    0x05
};

struct object_table {
	short num;
	
	uint32_t   mask[OBJ_SLOTS];
	uint32_t   id[OBJ_SLOTS];
	vec3_t     pos[OBJ_SLOTS];
	vec3_t     vel[OBJ_SLOTS];
	vec3_t     acl[OBJ_SLOTS];
	vec3_t     dir[OBJ_SLOTS];
	short      model[OBJ_SLOTS];
	short      anim[OBJ_SLOTS];
	float      prog[OBJ_SLOTS];
	mat4_t     mat[OBJ_SLOTS];
	int        len[OBJ_SLOTS];
	char       buf[OBJ_SLOTS][OBJ_DATA_MAX];

	int64_t    last[OBJ_SLOTS];
};


/* Define the global object-wrapper instance */
extern struct object_table objects;


/*
 * Initialize the global object-wrapper.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int obj_init(void);


/*
 * Close the object-table, delete all objects and free the allocated memory.
 */
extern void obj_close(void);


/*
 * Create a new object and add it to the model-table.
 *
 * @id: The id of the model
 * @mask: The mask for the model
 * @pos: The position of the model
 * @model: The slot of the model in the model-table
 * @data: A buffer containing additional data
 * @len: The length of the data-buffer in bytes
 *
 * Returns: The index of the object in the object-table
 */
extern short obj_set(uint32_t id, uint32_t mask, vec3_t pos, short model,
		char *data, int len);


/*
 * Remove an object and free the allocated memory.
 *
 * @slot: The slot of the object
 */
extern void obj_del(short slot);


/*
 * Modify an object.
 * 
 * @slot: The slot of the object in the table
 * @attr: The attribute to modify
 * @data: A buffer containing the data
 * @len: The length of the data-buffer in bytes
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int obj_mod(short slot, short attr, void *data, int len);


/*
 * Get an object by seaching for the id.
 *
 * @id: the id to search for
 *
 * Returns: The slot of the object with the id or -1 if an error occurred
 */
extern short obj_sel_id(uint32_t id);


/*
 * Update the model-matrix of an object
 *
 * @slot: The slot of the object
 */
extern void obj_update_matrix(short slot);


/*
 * Get a list of all object-ids and write them to the pointer. Note that the
 * first two written bytes are the number of objects.
 *
 * @ptr: Address to write the object-id-list to
 * @num: The number of objects written to the pointer
 * @max: The max-amount of objects to write to the pointer
 *
 * Returns: The number of bytes written to the pointer or -1 if an error
 * 	occurred
 */
extern int obj_list(void *ptr, short *num, short max);


/*
 * Get the data for the given ids.
 * Format:
 *  0 (1), [ID] (4), [Mask] (4), [Pos] (12), [Vel] (12), [Acl] (12)
 *
 *
 * @in: The list of ids to collect the data for
 * @in_num: The number of ids
 * @out: A ponter to attach the data to
 *
 * Returns: The number of bytes written to out or -1 if an error occurred
 */
extern int obj_collect(void *in, short in_num, void **out, short *out_num);


/*
 * Submit a single object into the object-list.
 *
 * @in: The object-buffer
 */
extern int obj_submit(void *in, int64_t ts);


/*
 * 
 */
extern int obj_collect_updates(void **out, short *out_num);


/*
 * Update the objects.
 */
extern int obj_update(void *in, short num);

/*
 * Print data about an object in the terminal.
 *
 * @slot: The slot of the object
 */
extern void obj_print(short slot);


/*
 * A system-function to update all objects in the object-table.
 *
 * @delt: The time since the last frame in milliseconds
 */
extern void obj_sys_update(float delt);


/*
 * Render all objects in the object-table.
 *
 * @interp: The interpolation-factor
 */
extern void obj_sys_render(float interp);

#endif
