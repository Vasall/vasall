#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "model.h"

typedef struct Object {
	/*
	 * The current position of the
	 * object.
	 */
	Vec3 pos;

	/*
	 * The current rotation of the 
	 * object.
	 */
	Vec3 rot;

	/*
	 * The current velocity of the
	 * object.
	 */
	Vec3 vel;

	/*
	 * The model of the object.
	 */
	Model *model;
} Object;


/* Create a new object and attach it to the object-array */
Object *objCreate(float x, float y, float z);

/* Destory an existing object */
void objDestory(Object *obj);

/* Render an object on the screen */
void objRender(Object *obj);

/* Get the current position of the object */
void objGetPos(Entity *ent, Vec3 pos);

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

/* Set the object-model */
void objSetModel(Object *obj, Model *mod);

#endif
