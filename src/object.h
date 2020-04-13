#ifndef _OBJECT_H_
#define _OBJECT_H_

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
extern struct ht_t *object_table;


/* Initialize the object-list */
int objInit(void);

/* Clear the object-list */
void objClose(void);

/* Create a new object and insert it into the object-list */
int objSet(char *key, char* mdl, vec3_t pos);

/* Destory an existing object and remove it from the object-list */
void objDel(char *key);

/* Get an object via the object-key */
struct object *objGet(char *key);

/* Update an object and adjust the attributes */
void objUpdate(struct object *obj, float delt);

/* Render an object on the screen */
void objRender(struct object *obj);

/* --------------------------------------- */
/*                POSITION                 */
/* --------------------------------------- */

/* Get the current position of the object */
void objGetPos(struct object *obj, vec3_t pos);

/* Set the new position of the object */
void objSetPos(struct object *obj, vec3_t pos);

/* Add a vector to the position */
void objAddPos(struct object *obj, vec3_t del);

/* --------------------------------------- */
/*                ROTATION                 */
/* --------------------------------------- */

/* Get the current rotation of the object */
void objGetRot(struct object *obj, vec3_t rot);

/* Set the new rotation of the object */
void objSetRot(struct object *obj, vec3_t rot);

/* Add a vector to the rotation */
void objAddRot(struct object *obj, vec3_t del);

/* --------------------------------------- */
/*                VELOCITY                 */
/* --------------------------------------- */

/* Get the current velocity of the object */
void objGetVel(struct object *obj, vec3_t vel);

/* Set the velocity of the object */
void objSetVel(struct object *obj, vec3_t vel);

/* Add a vector to the velocity */
void objAddVel(struct object *obj, vec3_t del);

/* --------------------------------------- */
/*                DIRECTION                */
/* --------------------------------------- */

/* Set the facing-direction of the object */
void objSetDir(struct object *obj, vec3_t dir);

/* Get the facing-direction of the object */
void objGetDir(struct object *obj, vec3_t dir);

/* --------------------------------------- */
/*                  MODEL                  */
/* --------------------------------------- */

/* Set the object-model */
void objSetModel(struct object *obj, char *mdl);

/* Get the model-matrix for rendering */
void objGetMatrix(struct object *obj, mat4_t mat);

/* Update model-matrix */
void objUpdMatrix(struct object *obj);

/* Output info about the object in the terminal */
void objPrint(struct object *obj);

#endif
