#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "vec.h"
#include "mat.h"
#include "list.h"
#include "model.h"
#include <stdint.h>

#define OBJ_LIMIT 128

struct object {
	/* 
	 * The id of this object in the object-array.
	 */
	uint16_t id;

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
	 * The model of the object.
	 */
	struct model *model;

	/*
	 * The model-matrix used for
	 * convert the vertices from
	 * model space to world space.
	 */
	Mat4 matrix;

	/*
	 * The current rotation of the 
	 * object.
	 */
	Vec3 rot;

	/*
	 * The vector to scale the object with.
	 */
	Vec3 scl;
};


/* The global object-list */
extern struct ptr_list *object_list;

/* Initialize the object-array */
int objInit(void);

/* Create a new object and attach it to the object-array */
struct object *objCreate(Vec3 pos);

/* Destory an existing object */
void objDestroy(struct object *obj);

/* Get an object via the object-id */
struct object *objGet(uint16_t id);

/* Update an object and adjust the attributes */
void objUpdate(struct object *obj);

/* Render an object on the screen */
void objRender(struct object *obj);

/* Get the current position of the object */
void objGetPos(struct object *obj, Vec3 pos);

/* Set the new position of the object */
void objSetPos(struct object *obj, Vec3 pos);

/* Add a vector to the position */
void objAddPos(struct object *obj, Vec3 del);

/* Get the current rotation of the object */
void objGetRot(struct object *obj, Vec3 rot);

/* Set the new rotation of the object */
void objSetRot(struct object *obj, Vec3 rot);

/* Add a vector to the rotation */
void objAddRot(struct object *obj, Vec3 del);

/* Get the current velocity of the object */
void objGetVel(struct object *obj, Vec3 vel);

/* Set the velocity of the object */
void objSetVel(struct object *obj, Vec3 vel);

/* Add a vector to the velocity*/
void objAddVel(struct object *obj, Vec3 del);

/* Set the object-model */
void objSetModel(struct object *obj, struct model *mod);

/* Get the model-matrix for rendering */
void objGetMatrix(struct object *obj, Mat4 mat);

/* Update model-matrix */
void objUpdMatrix(struct object *obj);

/* Output info about the object in the terminal */
void objPrint(struct object *obj);

#endif
