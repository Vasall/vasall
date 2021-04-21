#ifndef _OBJECT_H
#define _OBJECT_H

#include "extmath.h"
#include "vector.h"
#include "matrix.h"
#include "shape.h"
#include "sdl.h"
#include "asset.h"
#include "model.h"
#include "rig.h"
#include "input.h"
#include "controller.h"
#include "core.h"

#define OBJ_LIM      128
#define OBJ_DATA_MAX   128

/*
 * The different object-masks specifying behaviour and datahandling for the
 * objects.
 */
#define OBJ_M_NONE     0
#define OBJ_M_MODEL    (1<<0)  /* Attach and render a model for this object  */
#define OBJ_M_RIG      (1<<1)  /* Rig the model for animation and collision  */
#define OBJ_M_GRAV     (1<<2)  /*  */
#define OBJ_M_MOVE     (1<<3)  /* The object is movable                      */
#define OBJ_M_SOLID    (1<<4)  /* Should the object be solid with colliders  */
#define OBJ_M_SYNC     (1<<5)  /* Synchronize the object with other peers    */
#define OBJ_M_PEER     (1<<6)  /* The object is a peers player               */
#define OBJ_M_DATA     (1<<7)  /* A databuffer is attached to the object     */

/* Wrapper-masks */
#define OBJ_M_STATIC (OBJ_M_MODEL|OBJ_M_SOLID)
#define OBJ_M_ENTITY (OBJ_M_MODEL|OBJ_M_RIG|OBJ_M_GRAV|OBJ_M_MOVE|OBJ_M_SOLID|OBJ_M_SYNC)
#define OBJ_M_PLAYER (OBJ_M_ENTITY|OBJ_M_PEER)

/*
 * The different masks for the object-attributes.
 */
#define OBJ_A_ID       (1<<0)
#define OBJ_A_MASK     (1<<1)
#define OBJ_A_POS      (1<<2)
#define OBJ_A_VEL      (1<<3)
#define OBJ_A_MOV      (1<<4)
#define OBJ_A_DIR      (1<<5)
#define OBJ_A_BUF      (1<<6)

#define OBJ_A_ALL (OBJ_A_ID|OBJ_A_MASK|OBJ_A_POS|OBJ_A_VEL|OBJ_A_MOV|OBJ_A_BUF)


/*
 * The object-movement-log used to periodically store the objects current
 * position, velocity and input which will be used to correct a objects
 * movement by using a log-entry as a startpoint to reenact the movement of the
 * object.
 *
 * The movement-log allows new entries to be append at the end, but not inserted
 * in between. It also allows entries with the same timestamp to be updated with
 * new data after it has already been inserted.
 *
 * The used list is a rotating list which overwrites old data with new data if
 * the entry-limit has been reached, while keeping the elements in sorted order
 * from lowest timestamp to highest timestamp.
 */

#define OBJ_LOG_LIM 12
#define OBJ_LOG_TIME 80

struct obj_log {
	short start;
	short num;

	uint32_t  ts[OBJ_LOG_LIM];

	vec3_t    pos[OBJ_LOG_LIM];
	vec3_t    vel[OBJ_LOG_LIM];

	vec2_t    mov[OBJ_LOG_LIM];
	vec3_t    dir[OBJ_LOG_LIM];
};

struct comp_marker {
	uint32_t     ts;
	vec3_t       pos;
	vec3_t       vel;
	vec2_t       mov;
};

struct obj_collision {
	int mask;

	cube_t box;
	vec3_t min;
	vec3_t max;
};

struct obj_wrapper {
	short                    num;
	short                    order[OBJ_LIM];
	uint32_t                 last_ts;
	uint32_t                 rets;


	/* The object-mask and identification-number */
	uint32_t                 mask[OBJ_LIM];
	uint32_t                 id[OBJ_LIM];

	/* The runtime-buffers */
	uint32_t                 ts[OBJ_LIM];
	vec3_t                   pos[OBJ_LIM];
	vec3_t                   vel[OBJ_LIM];
	vec2_t                   mov[OBJ_LIM];
	vec3_t                   dir[OBJ_LIM];

	/* The save-buffers for the previous state */
	uint32_t                 prev_ts[OBJ_LIM];
	vec3_t                   prev_pos[OBJ_LIM];
	vec3_t                   prev_dir[OBJ_LIM];

	/* Buffer containing the runtime-log */
	struct obj_log           log[OBJ_LIM];

	/* Variables used for rendering and animation */
	vec3_t                   ren_pos[OBJ_LIM];
	vec3_t                   ren_dir[OBJ_LIM];

	/* The point the object is currently aiming at in world-space */
	vec3_t                   aim_origin[OBJ_LIM];
	vec3_t                   aim_pos[OBJ_LIM];

	/* The position and direction of the barrelt in world-space */
	vec3_t                   aim_off[OBJ_LIM];
	vec3_t                   aim_dir[OBJ_LIM];


	/* The model and the rig */
	short                    mdl[OBJ_LIM];
	struct model_rig         *rig[OBJ_LIM];

	/* The render-matrices */
	mat4_t                   mat_pos[OBJ_LIM];
	mat4_t                   mat_rot[OBJ_LIM];
	float                    vagl[OBJ_LIM][2]; /* 0: z-axis, 1: x-axis */

	/* Collision */
	struct obj_collision     col[OBJ_LIM]; 

	/* Object/Player-Data like Health and Mana */
	int                      len[OBJ_LIM];
	char                     data[OBJ_LIM][OBJ_DATA_MAX];
};


/* Define the global object-wrapper instance */
extern struct obj_wrapper g_obj;

/* TODO: This seems kinda funky tbh */
#include "object_utils.h"


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
 * @ts: The timestamp of the objects creation
 *
 * Returns: The index of the object in the object-table
 */
extern short obj_set(uint32_t id, uint32_t mask, vec3_t pos, short model,
		char *data, int len, uint32_t ts);


/*
 * Remove an object and free the allocated memory.
 *
 * @slot: The slot of the object
 */
extern void obj_del(short slot);


/*
 * Derive a rig from a model and attach it to the object.
 *
 * @slot: The slot of the object
 * @mdlslot: The slot of the model
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int obj_attach_rig(short slot, short mdlslot);


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
 * @out: A pointer to attach the object-data to
 * @out_num: A pointer to write the number of returned objects to
 *
 * Returns: The number of bytes written to out or -1 if an error occurred
 */
extern int obj_collect(uint16_t flg, void *in, short in_num, void **out,
		short *out_num);


/*
 * Submit a single object into the object-list.
 *
 * @in: The object-buffer
 * @ts: The timestamp of the current state of the object
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int obj_submit(void *in, uint32_t ts);


/*
 * Attach a new input to an object.
 *
 * @slot: The slot the objects is on in the object-table
 * @mask: The input-mask
 * @mov: The move-vector if enabled in the mask, use NULL otherwise
 * @act: The action-mask if enabled in the mask, use NULL otherwise
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int obj_add_input(short slot, uint32_t mask, uint32_t ts, vec2_t mov,
		uint16_t act);


/*
 * 
 */
extern int obj_update(void *in);


/*
 * 
 */
extern void obj_move(short slot);


/*
 * Print data about an object in the terminal.
 *
 * @slot: The slot of the object
 */
extern void obj_print(short slot);


/*
 * A system-function to update all objects in the object-table.
 *
 * @now: The current network time (milliseconds)
 */
extern void obj_sys_update(uint32_t now);


/*
 * Calculate the render-position of all objects.
 *
 * @interp: The interpolation-factor
 */
extern void obj_sys_prerender(float interp);

/*
 * Render the models attached to the objects on the screen.
 */
extern void obj_sys_render(void);

#endif /* _OBJECT_H */
