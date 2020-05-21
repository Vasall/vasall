#include "camera.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/* Redefine the global camera-wrapper */
struct camera_wrapper camera;


extern int cam_init(float aov, float asp, float near, float far)
{
	/* Set the default position of the camera */
	vec3_set(camera.pos, 0.0, 0.0, 0.0);

	/* Set the direction-vector for the camera */	
	vec3_set(camera.forward, 1.0, 0.0, 0.0);
	vec3_set(camera.right, 0.0, 0.0, 1.0);

	/* Set the sensitivity of the mouse */
	camera.sens = 0.5;

	/* Calculate the initial distance */
	camera.dist = vec3_mag(camera.pos);

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
	vec3_t up = {0.0, 1.0, 0.0};

	vec3_cpy(camera.forward, dir);
	vec3_nrm(camera.forward, camera.forward);

	vec3_cross(up, camera.forward, camera.right);

	cam_update_view();
}


extern void cam_get_dir(vec3_t dir)
{
	vec3_cpy(dir, camera.forward);
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
	vec3_t stdup = {0.0, 1.0, 0.0};

	d_yaw *= camera.sens;
	d_pitch *= camera.sens;

	if(d_yaw != 0.0) {
		vec3_rot_y(camera.forward, d_yaw, camera.forward);
	}

	if(d_pitch != 0.0) {
		vec3_rot_axes(camera.forward, d_pitch, 
				camera.right, camera.forward);
	}

	vec3_nrm(camera.forward, camera.forward);

	vec3_cross(stdup, camera.forward, camera.right);
	vec3_nrm(camera.right, camera.right);

	cam_update();
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
	vec3_t up = {0.0, 1.0, 0.0};

	if(camera.pos[0] == trg[0] && camera.pos[1] == trg[1] &&
			camera.pos[2] == trg[2]) {
		printf("Target equal to cam position!\n");
		vec3_set(camera.forward, 1.0, 0.0, 0.0);

		vec3_cross(up, camera.forward, camera.right);
		vec3_nrm(camera.right, camera.right);
		return;
	}

	vec3_sub(camera.pos, trg, camera.forward);
	vec3_nrm(camera.forward, camera.forward);

	vec3_cross(up, camera.forward, camera.right);
	vec3_nrm(camera.right, camera.right);
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
	vec3_t forw;
	vec3_t right;
	vec3_t up;
	vec3_t stdup;

	vec3_set(stdup, 0.0, 1.0, 0.0);

	vec3_cpy(forw, camera.forward);
	vec3_cpy(right, camera.right);

	vec3_cross(forw, right, up);

	camera.view[0x0] = right[0];
	camera.view[0x4] = right[1];
	camera.view[0x8] = right[2];

	camera.view[0x1] = up[0];
	camera.view[0x5] = up[1];
	camera.view[0x9] = up[2];

	camera.view[0x2] = forw[0];
	camera.view[0x6] = forw[1];
	camera.view[0xa] = forw[2];

	camera.view[0xc] = (-right[0] * camera.pos[0]) - 
		(right[1] * camera.pos[1]) - (right[2] * camera.pos[2]);
	camera.view[0xd] = (-up[0] * camera.pos[0]) -
		(up[1] * camera.pos[1]) - (up[2] * camera.pos[2]);
	camera.view[0xe] = (-forw[0] * camera.pos[0]) -
		(forw[1] * camera.pos[1]) - (forw[2] * camera.pos[2]);
}


extern void cam_set(vec3_t pos, vec3_t trg)
{	
	vec3_t up = {0.0, 1.0, 0.0};

	vec3_cpy(camera.pos, pos);

	vec3_sub(pos, trg, camera.forward);
	vec3_nrm(camera.forward, camera.forward);

	vec3_cross(up, camera.forward, camera.right);
	vec3_nrm(camera.right, camera.right);

	cam_update_view();
}


extern void cam_trg_obj(short obj)
{	
	camera.trg_obj = obj;
}


extern void cam_update(void)
{
	if(camera.trg_obj >= 0) {
		vec3_t pos;
		vec3_t tmp;

		vec3_cpy(pos, objects.pos[camera.trg_obj]);
		vec3_scl(camera.forward, camera.dist, tmp);
		vec3_add(pos, tmp, camera.pos);
	}

	cam_update_view();
}
