#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "vec.h"
#include "mat.h"
#include "model.h"

#define OBJ_LIMIT 128

typedef struct Object {
	/* The id of this object in the object-array */
	unsigned int id;

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
	Model *model;

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
} Object;

extern Object **object_array;
extern short object_number;

/* Initialize the object-array */
int objInit(void);

/* Create a new object and attach it to the object-array */
Object *objCreate(Vec3 pos);

/* Destory an existing object */
void objDestory(Object *obj);

/* Get an object via the object-id */
Object *objGet(uint32_t id);

/* Update an object and adjust the attributes */
void objUpdate(Object *obj);

/* Render an object on the screen */
void objRender(Object *obj);

/* Get the current position of the object */
void objGetPos(Object *obj, Vec3 pos);

/* Set the new position of the object */
void objSetPos(Object *obj, Vec3 pos);

/* Add a vector to the position */
void objAddPos(Object *obj, Vec3 del);

/* Get the current rotation of the object */
void objGetRot(Object *obj, Vec3 rot);

/* Set the new rotation of the object */
void objSetRot(Object *obj, Vec3 rot);

/* Add a vector to the rotation */
void objAddRot(Object *obj, Vec3 del);

/* Get the current velocity of the object */
void objGetVel(Object *obj, Vec3 vel);

/* Set the velocity of the object */
void objSetVel(Object *obj, Vec3 vel);

/* Add a vector to the velocity*/
void objAddVel(Object *obj, Vec3 del);

/* Set the object-model */
void objSetModel(Object *obj, Model *mod);

/* Get the model-matrix for rendering */
void objGetMatrix(Object *obj, Mat4 mat);

/* Update model-matrix */
void objUpdMatrix(Object *obj);

/* Output info about the object in the terminal */
void objPrint(Object *obj);

#endif
