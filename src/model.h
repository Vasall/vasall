#ifndef _MODEL_H_
#define _MODEL_H_

#include <stdint.h>
#include "vec.h"
#include "mat.h"
#include "shader.h"
#include "utils.h"

#define MESH_OK                  0
#define MESH_ERR_CREATING       -1
#define MESH_ERR_ADD_BAO        -2
#define MESH_ERR_SHADER         -3
#define MESH_ERR_FINISHING      -4

/*
 * A struct used to store data for
 * a single buffer-array-object.
 */
struct bao_entry {
	/*
	 * The index of the bao, with which
	 * it is bound to the vao.
	 */
	int index;

	/*
	 * A stack containing all data for this
	 * bao.
	 */
	void *buf;
	
	/* 
	 * Both the size of a single element and the
	 * number of elements in the data-buffer.
	 */
	int ele_size;
	int ele_num;

	/*
	 * The attribute-pointer or -1 if
	 * the bao is not bound.
	 */
	int attr_ptr;

	/*
	 * The name of the variable to bind
	 * the bao to.
	 */
	char attr_name[24];
};

/*
 * A struct used to store data about a single
 * model, which will then be pushed into the
 * model-cache, to enable crossreferncing from
 * multible objects at the same time.
 */
struct model {
	/*
	 * The vertex-array-object
	 * attached to the model.
	 */
	uint32_t vao;

	/*
	 * A list containing both the
	 * active index and the specific
	 * attr-pointer.
	 */
	struct dyn_stack *bao;

	/*
	 * The shader attached to this
	 * model.
	 */
	struct shader *shader;

	/*
	 * The status of the model.
	 */
	uint8_t status;
};


/* The global model-cache used to store all models */
extern struct model **model_cache;


/* Initialize the model-cache */
int mdlInit(void);

/* Close the model-cache */
void mdlClose(void);

/* Start creating a new model and fill struct with default values */
struct model *mdlCreate(void);

/* Finish creating the new model */
int mdlFinish(struct model *mdl);

/* Set the vertices and indices of the model */
void mdlSetMesh(struct model *mdl, Vec3 *vtxbuf, int vtxlen, 
		uint32_t *idxbuf, int idxlen, uint8_t nrmflg);

/* Attach a new buffer to the model */
void mdlAddBAO(struct model *mdl, uint8_t atype, void *buf, int elsize,
		int num, int8_t bflag, uint8_t bsize, uint8_t btype, 
		char *bname);

/* Render a model */
void mdlRender(struct model *mdl, Mat4 mat);

/* Create a red-cube as a model (used for dev) */
struct model *mdlRedCube(void);

#endif
