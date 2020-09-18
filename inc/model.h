#ifndef _MODEL_H
#define _MODEL_H

#include "vec.h"
#include "mat.h"
#include "sdl.h"
#include "asset.h"
#include "camera.h"
#include "amoloader/amoloader.h"

#define MDL_NAME_MAX            8
#define MDL_SLOTS             256

enum mdl_status {
	MDL_OK =                0,
	MDL_ERR_CREATING =      1,
	MDL_ERR_MESH =          2,
	MDL_ERR_TEXTURE =       3,
	MDL_ERR_LOADING =       4,
	MDL_ERR_SHADER =        5,
	MDL_ERR_FINISHING =     6
};

struct mdl_joint;
struct mdl_joint {
	char name[100];
	int par;
};

struct mdl_keyfr {
	float ts;

	float *pos;
	float *rot;
};

struct mdl_anim {
	char name[256];

	float dur;

	int               keyfr_num;
	struct mdl_keyfr  *keyfr_buf;
};

struct mdl_anim_wrapper {
	int                    joint_num;
	struct mdl_anim_joint  *joint_lst;
	float                  *joint_mat;

	int              num;
	struct mdl_anim  *lst;
};

enum mdl_type {
	MDL_BARE,
	MDL_ANIM
};

struct model {
	char          name[9];
	enum mdl_type type;
	unsigned int  vao;
	
	unsigned int  idx_bao;
	int           idx_num;
	unsigned int  *idx_buf;
	
	unsigned int  vtx_bao;
	int           vtx_num;
	float         *vtx_buf;

	short         tex;
	short         shd;

	int              jnt_num;
	struct mdl_joint *jnt_buf;
	float            *jnt_mat;

	int              anim_num;
	struct mdl_anim  *anim_buf;

	uint8_t       status;
};


/* Define the global model-wrapper instance */
extern struct model *models[MDL_SLOTS];


/*
 * Initialize the global model-table.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int mdl_init(void);


/*
 * Close the model-wrapper, remove all models and free the allocated memory.
 */
extern void mdl_close(void);


/* 
 * Create a new entry in the model-table.
 *
 * @name: The name of the model
 *
 * Returns: Either a slot in the model-table or -1 if an error occurred
 */
extern short mdl_set(char *name);


/*
 * Get the slot of a model in the table by searching for a name.
 *
 * @name: The name of the model
 *
 * Returns: Either the slot the model is on or -1 if an error occurred
 */
extern short mdl_get(char *name);


/*
 * Remove a model from the table, delete it and free the allocated memory.
 *
 * @slot: The slot of the model to delete
 */
extern void mdl_del(short slot);


/*
 * Attach data to a model.
 */
extern void mdl_set_data(short slot, int vtxnum, float *vtx, float *tex,
		float *nrm, unsigned int *jnt, float *wgt, int idxnum,
		unsigned int *idx);


/*
 * Attach a texture to a model.
 *
 * @slot: The slot of the model
 * @tex: The slot of the texture
 */
extern void mdl_set_texture(short slot, short tex);


/*
 * Attach a shader to a model.
 *
 * @slot: The slot of the model
 * @shd: The slot of the shader
 */
extern void mdl_set_shader(short slot, short shd);


/*
 * 
 */
extern short mdl_load(char *name, char *pth, short tex_slot, short shd_slot);

/*
 * Render a model with a model-matrix.
 *
 * @slot: The slot of the model to render
 * @mat_pos: The position-matrix
 * @mat_rot: The rotation-matrix
 */
extern void mdl_render(short slot, mat4_t mat_pos, mat4_t mat_rot);

#endif
