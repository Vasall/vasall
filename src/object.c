#include "object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Redefine external variables */
Object **object_array = NULL;
int object_number = 0;

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

	object_array = malloc(sizeof(Object *) * OBJ_LIMIT);
	if(object_array == NULL) return(-1);

	for(i = 0; i < OBJ_LIMIT; i++) {
		object_array[i] = NULL;
	}

	object_number = 0;

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

	/* Set the velocity */

	/* Set the direction */
	vecSet(obj->dir, 1.0, 1.0, 1.0);

	for(i = 0; i < OBJ_LIMIT; i++) {
		if(object_array[i] == NULL) {
			obj->id = (uint32_t)i;
			object_array[i] = obj;
			goto success;
		}
	}
	
failed:
	return(NULL);

success:
	obj->scl[0] = 1.0;
	obj->scl[1] = 1.0;
	obj->scl[2] = 1.0;

	objUpdMatrix(obj);
	
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
		if(object_array[i]->id == obj->id) {
			p = i;
			break;	
		}
	}

	if(p != -1) {
		free(object_array[p]);
		object_array[p] = NULL;
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
		if(object_array[i] != NULL && object_array[i]->id == id) {
			obj = object_array[i];
			break;
		}
	}

	return(obj);
}

/*
 * Update an object and use pyhsical calculations
 * to adjust the attributes of the object.
 *
 * @obj: Pointer to the object to update
 */
void objUpdate(Object *obj)
{
	
}

/* 
 * Render the model of an object on 
 * the screen.
 *
 * obj: Pointer to the object to render
*/
void objRender(Object *obj)
{
	mdlRender(obj->model, obj->matrix);
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
	objUpdMatrix(obj);
}

/* 
 * Set the position of the object and
 * copy the values of the given vector into
 * the position-vector of the object.
 *
 * @obj: Pointer to the object to set the position of
 * @pos: The new position of the object
 */
void objSetPos(Object *obj, Vec3 pos)
{
	vecCpy(obj->pos, pos);
	objUpdMatrix(obj);
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
	objUpdMatrix(obj);
}

/* 
 * Get the current rotation of the object
 * and copy the values to the given vector.
 *
 * @obj: Pointer to the object to get the rotation of
 * @rot: The vector to write the rotation-values to
*/
void objGetRot(Object *obj, Vec3 rot)
{
	vecCpy(rot, obj->rot);
	objUpdMatrix(obj);
}

/* 
 * Set the rotation of the object and
 * copy the values of the given vector to
 * the rotation-vector of the object.
 *
 * @obj: Pointer to the object to set the rotation of
 * @rot: The new rotation of the object
 */
void objSetRot(Object *obj, Vec3 rot)
{
	vecCpy(obj->rot, rot);
	objUpdMatrix(obj);
}

/*
 * Add a vector to the rotation by adding 
 * the values to the current rotation-vector
 * of the object.
 *
 * @obj: Pointer to the object to change the rotation of
 * @del: The vector to add to the rotation
*/
void objAddRot(Object *obj, Vec3 del)
{
	vecAdd(obj->rot, del, obj->rot);
	objUpdMatrix(obj);
}

/* 
 * Get the current velocity of the object
 * and copy the values to the given vector.
 *
 * @obj: Pointer to the object to get the velocity of
 * @vel: The vector to write the velocity-values to
*/
void objGetVel(Object *obj, Vec3 vel)
{
	vecCpy(vel, obj->vel);
}

/* 
 * Set the velocity of the object and
 * copy the values of the given vector to
 * the velocity-vector of the object.
 *
 * @obj: Pointer to the object to set the velocity of
 * @rot: The new velocity of the object
 */
void objSetVel(Object *obj, Vec3 vel)
{
	vecCpy(obj->vel, vel);
}

/*
 * Add a vector to the velocity by adding 
 * the values to the current velocity-vector
 * of the object.
 *
 * @obj: Pointer to the object to change the velocity of
 * @del: The vector to add to the velocity
*/
void objAddVel(Object *obj, Vec3 del)
{
	vecAdd(obj->vel, del, obj->vel);
}

/* 
 * Set the model of the object, by replacing
 * the current model-pointer with a new one.
 *
 * @obj: Pointer to the object to set the model of
 * @mod: Pointer to the model
 */
void objSetModel(Object *obj, Model *mod)
{
	obj->model = mod;
}

/*
 * Get the modelmatrix for rendering.
 * This function will combine all 3
 * model-matrices and return the result
 * as a new matrix.
 *
 * @obj: Pointer to the model
 * @mat: The matrix to write the model-matrix to
 */
void objGetMatrix(Object *obj, Mat4 mat)
{
	mat4Cpy(mat, obj->matrix);
}

/*
 * Update the model-matrix of a
 * given model.
 *
 * @obj: The model to update the mode-matrix for
 */
void objUpdMatrix(Object *obj)
{
	Vec3 rot;

	rot[0] = obj->rot[0] * TO_RADS;
	rot[1] = obj->rot[1] * TO_RADS;
	rot[2] = obj->rot[2] * TO_RADS;

	mat4Idt(obj->matrix);

	obj->matrix[0x0] =  cos(rot[1]);
	obj->matrix[0x2] = -sin(rot[1]);
	obj->matrix[0x8] =  sin(rot[1]);
	obj->matrix[0xa] =  cos(rot[1]);

	obj->matrix[0xc] = obj->pos[0];
	obj->matrix[0xd] = obj->pos[1];
	obj->matrix[0xe] = obj->pos[2];
}

/* 
 * Output info about the object in the terminal
 *
 * @obj: Pointer to the object to display data about
*/
void objPrint(Object *obj)
{
	printf("Id: %d\n", obj->id);
	printf("Pos: "); vecPrint(obj->pos); printf("\n");
	printf("Rot: "); vecPrint(obj->rot); printf("\n");
	printf("Vel: "); vecPrint(obj->vel); printf("\n");
}
