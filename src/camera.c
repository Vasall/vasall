#include "camera.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/* Redefine the global camera-wrapper */
struct cam_wrapper g_cam;


static vec3_t CAM_RIGHT =    {1.0, 0.0, 0.0};
static vec3_t CAM_FORWARD =  {0.0, 1.0, 0.0};
static vec3_t CAM_UP =       {0.0, 0.0, 1.0};

extern int cam_init(float aov, float asp, float near, float far)
{
	/* Set the initial mode of the camera */
	g_cam.mode = CAM_MODE_TPV;

	/* Set the default position of the camera */
	vec3_clr(g_cam.pos);

	/* Set the direction-vector for the camera */	
	vec3_cpy(g_cam.v_forward, CAM_FORWARD);
	vec3_cpy(g_cam.v_right,   CAM_RIGHT);

	/* Set the sensitivity of the mouse */
	g_cam.sens = 0.01;

	/* Calculate the initial distance */
	g_cam.dist = vec3_len(g_cam.pos);

	mat4_idt(g_cam.forw_m);

	/* Create the proj_mection matrix */
	mat4_idt(g_cam.proj_m);
	cam_proj_mat(aov, asp, near, far);

	/* Create the view-matrix */
	mat4_idt(g_cam.view_m);
	cam_update_view();

	g_cam.trg_obj = -1;
	return 0;
}


extern void cam_close(void)
{
	return;
}


extern void cam_get_proj(mat4_t mat)
{
	mat4_cpy(mat, g_cam.proj_m);
}


extern void cam_get_view(mat4_t mat)
{
	mat4_cpy(mat, g_cam.view_m);
}


extern void cam_set_pos(vec3_t pos)
{
	vec3_cpy(g_cam.pos, pos);
}


extern void cam_get_pos(vec3_t pos)
{
	vec3_cpy(pos, g_cam.pos);
}


extern void cam_set_dir(vec3_t dir)
{
	vec3_cpy(g_cam.v_forward, dir);
	vec3_nrm(g_cam.v_forward, g_cam.v_forward);

	vec3_cross(g_cam.v_forward, CAM_UP, g_cam.v_right);

	cam_update_view();
}


extern void cam_get_dir(vec3_t dir)
{
	vec3_cpy(dir, g_cam.v_forward);
}


extern void cam_zoom(int val)
{
	g_cam.dist += val;
	if(g_cam.dist < 0.5)
		g_cam.dist = 0.5;

	cam_update();
}


extern void cam_rot(float d_yaw, float d_pitch)
{
	d_yaw *= g_cam.sens;
	d_pitch *= g_cam.sens;

	if(d_yaw != 0.0) {
		vec3_rot_z(g_cam.v_forward, d_yaw, g_cam.v_forward);
	}

	if(d_pitch != 0.0) {
		vec3_rot_axes(g_cam.v_forward, d_pitch,
				g_cam.v_right, g_cam.v_forward);
	}

	/*
	 * Verify the pitch of the camera is in limits in FPV.
	 */
	if(g_cam.mode == CAM_MODE_FPV) {
		float agl;

		if(g_cam.v_forward[2] > 0) {	
			agl = asin(g_cam.v_forward[2]);
			agl = RAD_TO_DEG(agl);

			if(agl > CAM_PITCH_LIM)
				agl = CAM_PITCH_LIM;

			agl = DEG_TO_RAD(agl);
			g_cam.v_forward[2] = sin(agl);
		}
		else {
			agl = asin(g_cam.v_forward[2]);
			agl = RAD_TO_DEG(agl);

			if(agl < -CAM_PITCH_LIM)
				agl = -CAM_PITCH_LIM;

			agl = DEG_TO_RAD(agl);
			g_cam.v_forward[2] = sin(agl);
		}
	}

	vec3_nrm(g_cam.v_forward, g_cam.v_forward);

	vec3_cross(g_cam.v_forward, CAM_UP, g_cam.v_right);
	vec3_nrm(g_cam.v_right, g_cam.v_right);

	/*
	 * Calculate the forward-rotation-matrix.
	 */
	if(g_cam.mode == CAM_MODE_FPV) {
		float yaw;
		float pitch;
		vec3_t f;

		vec3_cpy(f, g_cam.v_forward);

		yaw = atan2(f[0], f[1]);
		pitch = -asin(f[2]);

		yaw = RAD_TO_DEG(yaw);
		pitch = RAD_TO_DEG(pitch);

		mat4_rfagl_s(g_cam.forw_m, pitch, 0, yaw);
	}
}


extern void cam_mov(vec3_t mov)
{
	if(g_cam.trg_obj >= 0)
		return;

	vec3_add(g_cam.pos, mov, g_cam.pos);
	cam_update_view();
}


extern void cam_look_at(vec3_t trg)
{
	if(vec3_cmp(g_cam.pos, trg)) {
		printf("Target equal to cam position!\n");
		vec3_cpy(g_cam.v_forward, CAM_FORWARD);

		vec3_cross(g_cam.v_forward, CAM_UP, g_cam.v_right);
		vec3_nrm(g_cam.v_right, g_cam.v_right);
		return;
	}

	vec3_sub(trg, g_cam.pos, g_cam.v_forward);
	vec3_nrm(g_cam.v_forward, g_cam.v_forward);

	vec3_cross(g_cam.v_forward, CAM_UP, g_cam.v_right);
	vec3_nrm(g_cam.v_right, g_cam.v_right);
}


extern void cam_proj_mat(float aov, float asp, float near, float far)
{
	float bottom;
	float top;
	float left;
	float right;
	float tangent;

	g_cam.aov = aov;
	g_cam.asp = asp;
	g_cam.near = near;
	g_cam.far = far;

	tangent = near * tan(aov * 0.5 * M_PI / 180);

	top = tangent;
	bottom = -top;
	right = top * asp;
	left = -right; 

	g_cam.proj_m[0x0] = (2 * near) / (right - left);
	g_cam.proj_m[0x5] = (2 * near) / (top - bottom); 	
	g_cam.proj_m[0x8] = (right + left) / (right - left); 
	g_cam.proj_m[0x9] = (top + bottom) / (top - bottom); 
	g_cam.proj_m[0xa] = -(far + near) / (far - near); 
	g_cam.proj_m[0xb] = -1; 
	g_cam.proj_m[0xe] = (-2 * far * near) / (far - near); 
	g_cam.proj_m[0xf] = 0;
}


extern void cam_update_view(void)
{
	vec3_t f;
	vec3_t r;
	vec3_t u;
	vec3_t p;	

	mat4_t m;

	mat4_t conv = {
		1.0,  0.0,  0.0,  0.0,
   		0.0,  1.0,  0.0,  0.0,
   		0.0,  0.0, -1.0,  0.0,
		0.0,  0.0,  0.0,  1.0
	};

	/* Copy the current position of the camera */
	vec3_cpy(p, g_cam.pos);

	/* Calculate the forward, right and up vector for the camera */
	if(g_cam.mode == CAM_MODE_FPV) {
		vec4_t tmp = {0, 1, 0};
		vec4_trans(tmp, g_cam.forw_m, tmp);

		vec3_cpy(f, tmp);
		vec3_nrm(f, f);

		vec3_cross(f, CAM_UP, r);
		vec3_nrm(r, r);
	}
	else {
		vec3_cpy(f, g_cam.v_forward);
		vec3_cpy(r, g_cam.v_right);
	}

	vec3_cross(r, f, u);
	vec3_nrm(u, u);


#if 0
	printf("VIEW\n");
	printf("forward: "); vec3_print(f); printf("\n");
	printf("right: ");   vec3_print(r); printf("\n");
	printf("up: ");      vec3_print(u); printf("\n");
	printf("pos: ");     vec3_print(p); printf("\n");
#endif

	/*
	 * Calculate the view-matrix.
	 * See the link for how this works:
	 * https://gamedev.stackexchange.com/a/181826
	 */

	mat4_idt(m);

	m[0x0] = r[0];
	m[0x4] = r[1];
	m[0x8] = r[2];

	m[0x1] = u[0];
	m[0x5] = u[1];
	m[0x9] = u[2];

	m[0x2] = f[0];
	m[0x6] = f[1];
	m[0xa] = f[2];

	mat4_mult(conv, m, m);

	mat4_idt(g_cam.view_m);
	g_cam.view_m[0xc] = -p[0];
	g_cam.view_m[0xd] = -p[1];
	g_cam.view_m[0xe] = -p[2];

	mat4_mult(m, g_cam.view_m, g_cam.view_m);
}


extern void cam_set(vec3_t pos, vec3_t trg)
{	
	vec3_cpy(g_cam.pos, pos);

	vec3_sub(pos, trg, g_cam.v_forward);
	vec3_nrm(g_cam.v_forward, g_cam.v_forward);

	vec3_cross(CAM_UP, g_cam.v_forward, g_cam.v_right);
	vec3_nrm(g_cam.v_right, g_cam.v_right);

	cam_update_view();
}


extern void cam_trg_obj(short obj)
{	
	g_cam.trg_obj = obj;
}


extern enum cam_mode cam_get_mode(void)
{
	return g_cam.mode;
}


extern void cam_tgl_mode(void)
{
	g_cam.mode = g_cam.mode == CAM_MODE_FPV ? CAM_MODE_TPV : CAM_MODE_FPV;
}


extern void cam_proc_input(void)
{
	if(g_cam.mode == CAM_MODE_FPV) {
		vec3_cpy(g_obj.dir[g_cam.trg_obj], g_cam.v_forward);
	}
}


extern void cam_update(void)
{
	if(g_cam.trg_obj >= 0) {
		if(g_cam.mode == CAM_MODE_TPV) {
			vec3_t pos;
			vec3_t tmp;

			vec3_cpy(pos, g_obj.pos[g_cam.trg_obj]);
			pos[2] += 1.8;

			vec3_scl(g_cam.v_forward, -g_cam.dist, tmp);
			vec3_add(pos, tmp, g_cam.pos);
		}
		else if(g_cam.mode == CAM_MODE_FPV) {
			vec3_t pos;

			vec3_cpy(pos, g_obj.pos[g_cam.trg_obj]);
			pos[2] += 1.8;

			vec3_cpy(g_cam.pos, pos);
		}
	}

	cam_update_view();
}
