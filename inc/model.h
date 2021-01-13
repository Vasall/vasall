#ifndef _MODEL_H
#define _MODEL_H

#include "vector.h"
#include "matrix.h"
#include "shape.h"
#include "sdl.h"
#include "asset.h"
#include "camera.h"
#include "rig.h"
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

struct mdl_col {
	cube_t         bp_col;

	sphere_t       ne_col;
	mat3_t         ne_cbs;

	int            cm_vtx_c;
	int            cm_tri_c;
	vec3_t         *cm_vtx;
	int3_t         *cm_idx;
	vec3_t         *cm_nrm;
	vec4_t         *cm_equ;
};

struct mdl_joint {
	/* The null-terminated name of the joint */
	char name[100];

	/* The index of the parent-joint in the joint-array */
	int par;

	/* The number of child-joints and their indices in the joint-array */
	int child_num;
	int child_buf[10];

	/* The rest-matrix of the joint relative to the parent */
	mat4_t loc_bind_mat;

	/* The rest-matrix relative to the model-origin */
	mat4_t bind_mat;

	/* The inverse-rest-matrix relative to the model-origin */
	mat4_t inv_bind_mat;
};

struct mdl_keyfr {
	float prog;

	vec3_t *pos;
	vec4_t *rot;
};

struct mdl_anim {
	char name[256];

	float dur;

	int               keyfr_num;
	struct mdl_keyfr  *keyfr_buf;
};

#define MDL_M_NONE 0
#define MDL_M_MDL AMO_M_MDL
#define MDL_M_RIG AMO_M_RIG
#define MDL_M_ANI AMO_M_ANI

#define MDL_M_CBP AMO_M_CBP
#define MDL_M_CNE AMO_M_CNE
#define MDL_M_CCM AMO_M_CCM
#define MDL_M_COL (MDL_M_CBP|AMO_M_CNE|AMO_M_CCM)

struct model {
	short             slot;
	char              name[9];
	uint32_t          attr_m;
	unsigned int      vao;
	
	unsigned int      idx_bao;
	int               idx_num;
	unsigned int      *idx_buf;
	
	unsigned int      vtx_bao;
	int               vtx_num;
	char              *vtx_buf;

	short             tex;
	short             shd;

	int               jnt_num;
	struct mdl_joint  *jnt_buf;
	int               jnt_root;

	int               anim_num;
	struct mdl_anim   *anim_buf;

	int               col_mask;
	struct mdl_col    col;

	uint8_t           status;
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
 * Check if a given slot-number is in range of the model-array.
 *
 * @slot: The slot-number to check
 *
 * Returns: 0 if slot-number is valid, 1 if not 
 */
extern int mdl_check_slot(short slot);


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
		float *nrm, int *jnt, float *wgt, int idxnum,
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
 * @rig: Pointer to a rig or NULL if the model has none
 */
extern void mdl_render(short slot, mat4_t mat_pos, mat4_t rot_mat,
		struct model_rig *rig);

#endif
