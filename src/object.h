#ifndef _OBJECT_H
#define _OBJECT_H

#include "vec.h"
#include "mat.h"
#include "list.h"
#include "model.h"

#include <stdint.h>

#define OBJ_SLOTS 16

/* 
 * A container used to manage a single object.
 * Each object will be later written to the
 * object-buffer.
 */
struct object {
	/* 
	 * The id of this object in the object-array.
	 */
	char key[5];

	/*
	 * The current position of the
	 * object.
	 */
	vec3_t pos;

	/*
	 * The current velocity of the
	 * object.
	 */
	vec3_t vel;

	/*
	 * The current facing-direction of
	 * the object.
	 */
	vec3_t dir;

	/*
	 * A pointer to the model attached
	 * to this object.
	 */
	struct model *model;

	/*
	 * The model-matrix used for convert 
	 * the vertices from model space to 
	 * world space.
	 */
	mat4_t matrix;

	/*
	 * The current rotation of the object. 
	 * Used when calculating the model-matrix 
	 * for this object.
	 */
	vec3_t rot;

	/*
	 * The vector to scale the object
	 * with. Used when calculating the
	 * model-matrix for this object.
	 */
	vec3_t scl;
};


/* The global object-list containing all active objects */
extern struct ht_t *objects;


/*
 * Initialize the global object-list.
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
int obj_init(void);

/*
 * Clear the object-list, destroy all objects and free the allocated memory.
 */
void obj_close(void);

/*
 * Create and initialize a new object and then attach it to the object-list.
 *
 * @key: The key of the object
 * @mdl: A pointer to the model for the object
 * @pos: The position to place the object
 */
int obj_set(char *key, char* mdl, vec3_t pos);

/*
 * Delete an object and remove it from the object-list.
 *
 * @key: The key of the object
 */
void obj_del(char *key);

/* 
 * Get an object via the object-id.
 *
 * key: The key of this object
 *
 * Returns: Either a pointer to the object or NULL if an error occurred
*/
struct object *obj_get(char *key);

/*
 * Update an object and use pyhsical calculations to adjust the attributes of 
 * the object.
 *
 * @obj: Pointer to the object to update
 * @delt: The time since the last frame
 */
void obj_update(struct object *obj, float delt);

/* 
 * Render the model of an object on the screen.
 *
 * obj: Pointer to the object to render
*/
void obj_render(struct object *obj);

/* --------------------------------------- */
/*                POSITION                 */
/* --------------------------------------- */

/* 
 * Set the position of the object and copy the values of the given vector into
 * the position-vector of the object.
 *
 * @obj: Pointer to the object to set the position of
 * @pos: The new position of the object
 */
void obj_set_pos(struct object *obj, vec3_t pos);

/* 
 * Get the current position of the object and copy the values to the given 
 * vector.
 *
 * @obj: Pointer to the object to get the position of
 * @pos: The vector to copy the values into
*/
void obj_get_pos(struct object *obj, vec3_t pos);

/* 
 * Add a vector to the position by adding the values to the current 
 * position-vector of the object.
 *
 * @obj: Pointer to the object to change the position of
 * @vec: The vector to add to the position-vector
 */
void obj_add_pos(struct object *obj, vec3_t del);

/* --------------------------------------- */
/*                ROTATION                 */
/* --------------------------------------- */

/* 
 * Set the rotation of the object and copy the values of the given vector to
 * the rotation-vector of the object.
 *
 * @obj: Pointer to the object to set the rotation of
 * @rot: The new rotation of the object
 */
void obj_set_rot(struct object *obj, vec3_t rot);

/* 
 * Get the current rotation of the object and copy the values to the 
 * given vector.
 *
 * @obj: Pointer to the object to get the rotation of
 * @rot: The vector to write the rotation-values to
*/
void obj_get_rot(struct object *obj, vec3_t rot);

/*
 * Add a vector to the rotation by adding the values to the current 
 * rotation-vector of the object.
 *
 * @obj: Pointer to the object to change the rotation of
 * @del: The vector to add to the rotation
*/
void obj_add_rot(struct object *obj, vec3_t del);

/* --------------------------------------- */
/*                VELOCITY                 */
/* --------------------------------------- */

/* 
 * Set the velocity of the object and copy the values of the given vector into
 * the velocity-vector of the object.
 *
 * @obj: Pointer to the object to set the velocity of
 * @rot: The new velocity of the object
 */
void obj_set_vel(struct object *obj, vec3_t vel);

/* 
 * Get the current velocity of the object and copy the values into the 
 * given vector.
 *
 * @obj: Pointer to the object to get the velocity of
 * @vel: The vector to write the velocity-values to
*/
void obj_get_vel(struct object *obj, vec3_t vel);

/*
 * Add a vector to the velocity by adding the values to the current 
 * velocity-vector of the object.
 *
 * @obj: Pointer to the object to change the velocity of
 * @del: The vector to add to the velocity
*/
void obj_add_vel(struct object *obj, vec3_t del);

/* --------------------------------------- */
/*                DIRECTION                */
/* --------------------------------------- */

/* 
 * Set the facing-direction of the object and copy the given vector-values 
 * in the direction-vector.
 *
 * @obj: Pointer to the object
 * @dir: The new direction-vector
 */
void obj_set_dir(struct object *obj, vec3_t dir);

/* 
 * Get the facing-direction of the object and copy the values into the 
 * given vector.
 * 
 * @obj: Pointer to the object
 * @dir: A vector to copy the values into
 */
void obj_get_dir(struct object *obj, vec3_t dir);

/* --------------------------------------- */
/*                  MODEL                  */
/* --------------------------------------- */

/* 
 * Set the model of the object, by replacing the current model-pointer with 
 * a new one.
 *
 * @obj: Pointer to the object to set the model of
 * @mdl: The key of the model to attach
 */
void obj_set_model(struct object *obj, char *mdl);

/*
 * Get the modelmatrix for rendering. This function will combine all 3 
 * model-matrices and return the result as a new matrix.
 *
 * @obj: Pointer to the model
 * @mat: The matrix to write the model-matrix to
 */
void obj_get_mat(struct object *obj, mat4_t mat);

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
