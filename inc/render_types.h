#ifndef _RENDER_TYPES_H
#define _RENDER_TYPES_H

#include "rig.h"

enum mdl_type {
	MDL_TYPE_DEFAULT,
	MDL_TYPE_SKYBOX
};

struct uni_buffer {
	mat4_t pos_mat;
	mat4_t rot_mat;
	mat4_t view;
	mat4_t proj;
	mat4_t trans_mat[JOINT_MAX_NUM];
};

#endif
