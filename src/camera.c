#include "camera.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/* Redefine the global camera-wrapper */
struct camera_wrapper camera;


static vec3_t CAM_RIGHT = {1.0, 0.0, 0.0};
static vec3_t CAM_FORWARD =  {0.0, 1.0, 0.0};
static vec3_t CAM_UP =    {0.0, 0.0, 1.0};

extern int cam_init(float aov, float asp, float near, float far)
{
	/* Set the initial mode of the camera */
	camera.mode = CAM_MODE_TPV;

	/* Set the default position of the camera */
	vec3_clr(camera.pos);

	/* Set the direction-vector for the camera */	
	vec3_cpy(camera.v_forward, CAM_FORWARD);
	vec3_cpy(camera.v_right,   CAM_RIGHT);

	/* Set the sensitivity of the mouse */
	camera.sens = 0.01;

	/* Calculate the initial distance */
	camera.dist = vec3_len(camera.pos);

	/* Create the projection matrix */
	mat4_idt(camera.proj);
	cam_proj_mat(aov, asp, near, far);

	/* Create the view-matrix */
	mat4_idt(camera.view);
	cam_update_view();

	camera.trg_obj = -1;
	return 0;
}


extern void cam_close(void)
{
	return;
}


extern void cam_get_proj(mat4_t mat)
{
	mat4_cpy(mat, camera.proj);
}


extern void cam_get_view(mat4_t mat)
{
	mat4_cpy(mat, camera.view);
}


extern void cam_set_pos(vec3_t pos)
{
	vec3_cpy(camera.pos, pos);
}


extern void cam_get_pos(vec3_t pos)
{
	vec3_cpy(pos, camera.pos);
}


extern void cam_set_dir(vec3_t dir)
{
	vec3_cpy(camera.v_forward, dir);
	vec3_nrm(camera.v_forward, camera.v_forward);

	vec3_cross(camera.v_forward, CAM_UP, camera.v_right);

	cam_update_view();
}


extern void cam_get_dir(vec3_t dir)
{
	vec3_cpy(dir, camera.v_forward);
}


extern void cam_zoom(int val)
{
	camera.dist += val;
	if(camera.dist < 0.5)
		camera.dist = 0.5;

	cam_update();
}


extern void cam_rot(float d_yaw, float d_pitch)
{
	d_yaw *= -camera.sens;
	d_pitch *= camera.sens;

	if(camera.mode == CAM_MODE_TPV) {
		d_yaw = -d_yaw;
	}

	if(d_yaw != 0.0) {
		vec3_rot_z(camera.v_forward, d_yaw, camera.v_forward);
	}

	if(d_pitch != 0.0) {
		vec3_rot_axes(camera.v_forward, d_pitch,
				camera.v_right, camera.v_forward);
	}

	/*
	 * Verify the pitch of the camera is in limits in FPV.
	 */
	if(camera.mode == CAM_MODE_FPV) {
		float agl;

		if(camera.v_forward[2] > 0) {	
			agl = asin(camera.v_forward[2]);
			agl = RAD_TO_DEG(agl);

			if(agl > CAM_PITCH_LIM)
				agl = CAM_PITCH_LIM;

			agl = DEG_TO_RAD(agl);
			camera.v_forward[2] = sin(agl);
		}
		else {
			agl = asin(camera.v_forward[2]);
			agl = RAD_TO_DEG(agl);

			if(agl < -CAM_PITCH_LIM)
				agl = -CAM_PITCH_LIM;

			agl = DEG_TO_RAD(agl);
			camera.v_forward[2] = sin(agl);
		}

		vec3_nrm(camera.v_forward, camera.v_forward);
	}

	vec3_nrm(camera.v_forward, camera.v_forward);

	vec3_cross(camera.v_forward, CAM_UP, camera.v_right);
	vec3_nrm(camera.v_right, camera.v_right);
}


extern void cam_mov(vec3_t mov)
{
	if(camera.trg_obj >= 0)
		return;

	vec3_add(camera.pos, mov, camera.pos);
	cam_update_view();
}


extern void cam_look_at(vec3_t trg)
{
	if(vec3_cmp(camera.pos, trg)) {
		printf("Target equal to cam position!\n");
		vec3_cpy(camera.v_forward, CAM_FORWARD);

		vec3_cross(camera.v_forward, CAM_UP, camera.v_right);
		vec3_nrm(camera.v_right, camera.v_right);
		return;
	}

	vec3_sub(trg, camera.pos, camera.v_forward);
	vec3_nrm(camera.v_forward, camera.v_forward);

	vec3_cross(camera.v_forward, CAM_UP, camera.v_right);
	vec3_nrm(camera.v_right, camera.v_right);
}


extern void cam_proj_mat(float aov, float asp, float near, float far)
{
	float bottom;
	float top;
	float left;
	float right;
	float tangent;

	camera.aov = aov;
	camera.asp = asp;
	camera.near = near;
	camera.far = far;

	tangent = near * tan(aov * 0.5 * M_PI / 180);

	top = tangent;
	bottom = -top;
	right = top * asp;
	left = -right; 

	camera.proj[0x0] = (2 * near) / (right - left);
	camera.proj[0x5] = (2 * near) / (top - bottom); 	
	camera.proj[0x8] = (right + left) / (right - left); 
	camera.proj[0x9] = (top + bottom) / (top - bottom); 
	camera.proj[0xa] = -(far + near) / (far - near); 
	camera.proj[0xb] = -1; 
	camera.proj[0xe] = (-2 * far * near) / (far - near); 
	camera.proj[0xf] = 0;
}


extern void cam_update_view(void)
{
	vec3_t forward;
	vec3_t right;
	vec3_t up;

	vec3_cpy(forward, camera.v_forward);
	vec3_cpy(right,   camera.v_right);

	vec3_flip(forward, forward);
	vec3_flip(right, right);

	vec3_cross(right, forward, up);

#if 0
	printf("VIEW\n");
	printf("forward: "); vec3_print(forward); printf("\n");
	printf("right: ");   vec3_print(right);   printf("\n");
	printf("up: ");      vec3_print(up);      printf("\n");
#endif

	camera.view[0x0] = right[0];
	camera.view[0x4] = right[1];
	camera.view[0x8] = right[2];

	camera.view[0x1] = up[0];
	camera.view[0x5] = up[1];
	camera.view[0x9] = up[2];

	camera.view[0x2] = forward[0];
	camera.view[0x6] = forward[1];
	camera.view[0xa] = forward[2];

	camera.view[0xc] = (-right[0] * camera.pos[0]) - 
		(right[1] * camera.pos[1]) - (right[2] * camera.pos[2]);
	camera.view[0xd] = (-up[0] * camera.pos[0]) -
		(up[1] * camera.pos[1]) - (up[2] * camera.pos[2]);
	camera.view[0xe] = (-forward[0] * camera.pos[0]) -
		(forward[1] * camera.pos[1]) - (forward[2] * camera.pos[2]);
}


extern void cam_set(vec3_t pos, vec3_t trg)
{	
	vec3_cpy(camera.pos, pos);

	vec3_sub(pos, trg, camera.v_forward);
	vec3_nrm(camera.v_forward, camera.v_forward);

	vec3_cross(CAM_UP, camera.v_forward, camera.v_right);
	vec3_nrm(camera.v_right, camera.v_right);

	cam_update_view();
}


extern void cam_trg_obj(short obj)
{	
	camera.trg_obj = obj;
}


extern void cam_tgl_view(void)
{
	camera.mode = camera.mode == CAM_MODE_FPV ? CAM_MODE_TPV : CAM_MODE_FPV;
}


extern void cam_proc_input(void)
{
	vec3_cpy(objects.dir[camera.trg_obj], camera.v_forward);
}


extern void cam_update(void)
{
	if(camera.trg_obj >= 0) {
		if(camera.mode == CAM_MODE_TPV) {
			vec3_t pos;
			vec3_t tmp;

			vec3_cpy(pos, objects.ren_pos[camera.trg_obj]);
			pos[2] += 1.8;

			vec3_scl(camera.v_forward, -camera.dist, tmp);
			vec3_add(pos, tmp, camera.pos);
		}
		else if(camera.mode == CAM_MODE_FPV) {
			vec3_t pos;
			vec3_t tmp;

			vec3_cpy(pos, objects.ren_pos[camera.trg_obj]);
			pos[2] += 1.8;

			vec3_cpy(camera.pos, pos);
		}
	}

	cam_update_view();
}
