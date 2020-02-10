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
	Vec3 pos;

	/*
	 * The current velocity of the
	 * object.
	 */
	Vec3 vel;

	/*
	 * The current facing-direction of
	 * the object.
	 */
	Vec3 dir;

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
	Mat4 matrix;

	/*
	 * The current rotation of the object. 
	 * Used when calculating the model-matrix 
	 * for this object.
	 */
	Vec3 rot;

	/*
	 * The vector to scale the object
	 * with. Used when calculating the
	 * model-matrix for this object.
	 */
	Vec3 scl;
};


/* The global object-list containing all active objects */
extern struct ht_t *object_table;


/* Initialize the object-list */
int objInit(void);

/* Clear the object-list */
void objClose(void);

/* Create a new object and insert it into the object-list */
int objSet(char *key, char* mdl, Vec3 pos);

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
void objGetPos(struct object *obj, Vec3 pos);

/* Set the new position of the object */
void objSetPos(struct object *obj, Vec3 pos);

/* Add a vector to the position */
void objAddPos(struct object *obj, Vec3 del);

/* --------------------------------------- */
/*                ROTATION                 */
/* --------------------------------------- */

/* Get the current rotation of the object */
void objGetRot(struct object *obj, Vec3 rot);

/* Set the new rotation of the object */
void objSetRot(struct object *obj, Vec3 rot);

/* Add a vector to the rotation */
void objAddRot(struct object *obj, Vec3 del);

/* --------------------------------------- */
/*                VELOCITY                 */
/* --------------------------------------- */

/* Get the current velocity of the object */
void objGetVel(struct object *obj, Vec3 vel);

/* Set the velocity of the object */
void objSetVel(struct object *obj, Vec3 vel);

/* Add a vector to the velocity */
void objAddVel(struct object *obj, Vec3 del);

/* --------------------------------------- */
/*                DIRECTION                */
/* --------------------------------------- */

/* Set the facing-direction of the object */
void objSetDir(struct object *obj, Vec3 dir);

/* Get the facing-direction of the object */
void objGetDir(struct object *obj, Vec3 dir);

/* --------------------------------------- */
/*                  MODEL                  */
/* --------------------------------------- */

/* Set the object-model */
void objSetModel(struct object *obj, char *mdl);

/* Get the model-matrix for rendering */
void objGetMatrix(struct object *obj, Mat4 mat);

/* Update model-matrix */
void objUpdMatrix(struct object *obj);

/* Output info about the object in the terminal */
void objPrint(struct object *obj);

#endif
