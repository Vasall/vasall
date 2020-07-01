#include "update.h"

#include <stdlib.h>

void game_proc_evt(SDL_Event *evt)
{
	int mod_ctrl;
	uint8_t axis;
	float val;

	if(mod_ctrl) {/* Prevent warning for not using mod_ctrl */}

	switch(evt->type) {
		case(SDL_CONTROLLERAXISMOTION):
			axis = evt->caxis.axis;
			val = evt->caxis.value * 0.000001;

			if(axis == 0) {
				input.movement[0] = val;
			}
			else if(axis == 1) {
				input.movement[1] = val;
			}

			else if(axis == 2) {
				input.camera[0] = val;
			}
			else if(axis == 3) {
				input.camera[1] = val;
			}

			break;

		case(SDL_MOUSEWHEEL):
			cam_zoom(evt->wheel.y);
			break;

		case(SDL_MOUSEMOTION):
			if(evt->motion.state == SDL_BUTTON_LMASK) {
				/* If left mouse button pressed */
				cam_rot(-evt->motion.xrel / 50.0,
						-evt->motion.yrel / 50.0);
			}
			break;
		/* 
		case(XSDL_KEYDOWN):
			mod = evt->key.keysym.mod;
			mod_ctrl = mod & (KMOD_LCTRL | KMOD_RCTRL);
			switch(evt->key.keysym.sym) {
				case(SDLK_w):
					camMovDir(FORWARD);
					break;
				case(XSDLK_s):
					camMovDir(BACK);
					break;
				case(XSDLK_a):
					camMovDir(LEFT);
					break;
				case(XSDLK_d):
					camMovDir(RIGHT);
					break;
				case(XSDLK_q):
					camZoom(1);
					break;
				case(XSDLK_e):
					camZoom(-1);
					break;
				case(XSDLK_UP):
					camRot(0.0, -0.1);
					break;
				case(XSDLK_DOWN):
					camRot(0.0, 0.1);
					break;
				case(XSDLK_RIGHT):
					camRot(0.1, 0.0);
					break;
				case(XSDLK_LEFT):
					camRot(-0.1, 0.0);
					break;
			}
			break;
		*/
	}
}

void game_update(void)
{
	vec3_t vel, forw, right;

	/* Update network */
	net_update();

	/* Rotate the camera */	
	cam_rot(input.camera[0], input.camera[1]);

	/* Set object-acceleration */
	vec3_cpy(forw, camera.forward);
	forw[1] = 0.0;
	vec3_nrm(forw, forw);

	vec3_cpy(right, camera.right);
	right[1] = 0.0;
	vec3_nrm(right, right);

	vec3_scl(forw, input.movement[1], forw);
	vec3_scl(right, input.movement[0], right);

	vec3_add(forw, right, vel);

	vec3_cpy(objects.vel[core.obj], vel);

	obj_sys_update(5.0);

	/* Update the camera-position */
	cam_update();
}

void game_render(void)
{
	/* Render the world */
	wld_render();

	/* Render the objects */
	obj_sys_render();
}
