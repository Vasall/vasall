#ifndef _MODEL_H_
#define _MODEL_H_

#include <stdint.h>
#include "vec.h"
#include "mat.h"
#include "texture.h"
#include "shader.h"
#include "utils.h"

#define MDL_KEY_LEN             5
#define MDL_SLOTS               8

#define MDL_OK                  0
#define MDL_ERR_CREATING        1
#define MDL_ERR_MESH            2
#define MDL_ERR_TEXTURE         3
#define MDL_ERR_LOADING         4
#define MDL_ERR_SHADER          5
#define MDL_ERR_FINISHING       6

/*
 * A struct used to store data about a single
 * model, which will then be pushed into the
 * model-cache, to enable crossreferncing from
 * multible objects at the same time.
 */
struct model {
	/*
	 * The key of this model.
	 */
	char key[8];
	
	/*
	 * The vertex-array-object
	 * attached to the model.
	 */
	uint32_t vao;

	/*
	 * The index-bao, index-buffer and
	 * number of indices in the buffer.
	 */
	uint32_t idx_bao;
	int idx_num;
	int *idx_buf;

	/*
	 * The number of vertices attached
	 * to this model.
	 */
	int vtx_num;

	/*
	 * The vertex-buffer and
	 * vertex-bao.
	 */
	uint32_t vtx_bao;
	Vec3 *vtx_buf;

	/*
	 * The normal-buffer and
	 * normal-bao.
	 */
	uint32_t nrm_bao;
	Vec3 *nrm_buf;

	/*
	 * The uv-buffer and
	 * uv-bao.
	 */
	uint32_t uv_bao;
	Vec2 *uv_buf;


	/*
	 * The texture attached to this
	 * model.
	 */
	struct texture *tex;

	/*
	 * The shader attached to this
	 * model.
	 */
	struct shader *shader;

	/*
	 * The current status of the model.
	 */
	uint8_t status;
};


/* The global model-table used to store all models */
extern struct ht_t *model_table;


/* Create and nitialize the model-table */
int mdlInit(void);

/* Destroy and close the model-table */
void mdlClose(void);

/* Create a new model */
struct model *mdlCreate(char *key);

/* Attach a mesh to the model */
void mdlSetMesh(struct model *mdl, int idxnum, int *idx, 
		int vtxnum, Vec3 *vtx, Vec3 *nrm, Vec2 *uv);

/* Attach a texture to the model */
void mdlSetTex(struct model *mdl, char *tex);

/* Attach a shader to the model */
void mdlSetShader(struct model *mdl, char *shd);

/* Finish the model and push it into the model-table */
int mdlFinish(struct model *mdl);

/* Create a new model and push it into the model-table */
int mdlLoad(char *key, char *obj, char *tex, char *shd);

/* Get a model from the model-table using a key */
struct model *mdlGet(char *key);

/* Delete a model and remove it from the model-table */
void mdlDel(char *key);

/* Render a model using a given model-matrix */
void mdlRender(struct model *mdl, Mat4 mat);

int mdlLoadObj(char *pth, int *idxnum, int **idx, int *vtxnum,
		Vec3 **vtx, Vec3 **nrm, Vec2 **uv);
#endif
