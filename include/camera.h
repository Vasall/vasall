#ifndef _CAMERA_H
#define _CAMERA_H

#include "defines.h"
#include "camera.h"
#include "vec.h"
#include "mat.h"
#include "object.h"

typedef enum {LEFT, RIGHT, FORWARD, BACK} Direction;

struct camera_wrapper {
	vec3_t pos;
	vec3_t dir;
	vec3_t forward;
	vec3_t right;

	float aov;
	float asp;
	float near;
	float far;

	float dist;
	short trg_obj;

	mat4_t proj;
	mat4_t view;
	
	float sens;
};


extern struct camera_wrapper *camera;


extern int cam_init(float aov, float asp, float near, float far);
extern void cam_close(void);

extern void cam_get_proj(mat4_t mat);
extern void cam_get_view(mat4_t mat);

extern void cam_set_pos(vec3_t pos);
extern void cam_get_pos(vec3_t pos);

extern void cam_set_dir(vec3_t dir);
extern void cam_get_dir(vec3_t dir);

extern void cam_zoom(int val);
extern void cam_rot(float d_yaw, float d_pitch);
extern void cam_mov_dir(Direction dir);
extern void cam_mov(vec3_t mov);
extern void cam_look_at(vec3_t trg);

extern void cam_proj_mat(float aov, float asp, float near, float far);
extern void cam_update_view(void);

extern void cam_set(vec3_t pos, vec3_t trg);
extern void cam_trg_obj(short obj);

extern void cam_update(void);

#endif
