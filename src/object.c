#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Redefine external variables */
Object **objects;

/* 
 * Initialize the object-array and allocate
 * the necessary memory.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
*/
int objInit(void)
{
	int i;

	objects = malloc(sizeof(Object *) * OBJ_LIMIT);
	if(objects == NULL) return(-1);

	for(i = 0; i < OBJ_LIMIT; i++) {
		objects[i] = NULL;
	}

	return(0);
}

/* 
 * Create a new object at the given position and 
 * insert it into  the object-array, if there is 
 * still space left. If the limit has been
 * met, return NULL.
 *
 * @pos: The position to create the object at
 *
 * Returns: Either a pointer to the created object
 * 	or NULL if an error occurred
*/
Object *objCreate(Vec3 pos)
{
	int i;
	Object *obj;

	obj = calloc(1, sizeof(Object));
	if(obj == NULL) goto failed;

	/* Set the position */
	vecCpy(obj->pos, pos);

	for(i = 0; i < OBJ_LIMIT; i++) {
		if(objects[i] == NULL) {
			obj->id = (uint32_t)i;
			objects[i] = obj;
			goto success;
		}
	}

failed:
	return(NULL);

success:
	return(obj);	
}

/* Destory an existing object and remove
 * it from the object-array. Then free the
 * allocated memory.
 *
 * @obj: Pointer to the object to destroy
*/
void objDestory(Object *obj)
{
	int i, p = -1;

	if(obj != NULL) return;
		
	for(i = 0; i < OBJ_LIMIT; i++) {
		if(objects[i]->id == obj->id) {
			p = i;
			break;	
		}
	}

	if(p != -1) {
		free(objects[p]);
		objects[p] = NULL;
	}
}

/* 
 * Get an object via the object-id.
 *
 * id: The id of the object to search for
 *
 * Returns: Either the object with the given id
 * 	or NULL if an error occurred
*/
Object *objGet(uint32_t id)
{
	int i;
	Object *obj = NULL;

	for(i = 0; i < OBJ_LIMIT; i++) {
		if(objects[i] != NULL && objects[i]->id == id) {
			obj = objects[i];
			break;
		}
	}

	return(obj);
}

/* 
 * Render the model of an object on 
 * the screen.
 *
 * obj: Pointer to the object to render
*/
void objRender(Object *obj)
{
	mdlRender(obj->model);	
}

/* 
 * Get the current position of the object
 * and copy the values to the given vector.
 *
 * @obj: Pointer to the object to get the position of
 * @pos: The vector to copy the values into
*/
void objGetPos(Object *obj, Vec3 pos)
{
	vecCpy(pos, obj->pos);
}

/* 
 * Set the new position of the object and
 * copy the values of the given vector into
 * the position-vector of the object.
 *
 * @obj: Pointer to the object to set the position of
 * @pos: The new position of the object
 */
void objSetPos(Object *obj, Vec3 pos)
{
	vecCpy(obj->pos, pos);
}

/* 
 * Add a vector to the position by adding
 * the values to the current position-vector
 * of the object.
 *
 * @obj: Pointer to the object to change the position of
 * @vec: The vector to add to the position-vector
 */
void objAddPos(Object *obj, Vec3 del)
{
	vecAdd(obj->pos, del, obj->pos);
}

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
