#include "handle.h"
#include "object.h"
#include "input.h"

/*
 * This function is used as a
 * callback-function to handle
 * resizing the window.
 *
 * @evt: A pointer to the event
 */
V_API void handle_resize(XSDL_Event *evt)
{
	double ratio;
	XSDL_UIContext *ctx = window->ui_ctx;

	if(evt){/* Prevent warning for not using evt */}

	printf("%d/%d\n", ctx->win_w, ctx->win_h);

	/* Update the viewport */
	glViewport(0, 0, ctx->win_w, ctx->win_h);

	/* Update projection matrix */
	ratio = (double)ctx->win_w / (double)ctx->win_h;
	cam_proj_mat(45.0, ratio, 0.1, 1000.0);
}


V_API void menu_procevt(XSDL_Event *evt)
{
	if(evt) {/* Prevent warning for not using evt */}
}

V_API void menu_update(void)
{
}

V_API void game_procevt(XSDL_Event *evt)
{
	int mod_ctrl;
	uint8_t axis;
	float val;

	if(mod_ctrl) {/* Prevent warning for not using mod_ctrl */}

	switch(evt->type) {
		case(XSDL_CONTROLLERAXISMOTION):
			axis = evt->caxis.axis;
			val = evt->caxis.value * 0.000001;

			if(axis == 0) {
				inp_map->movement[0] = val;
			}
			else if(axis == 1) {
				inp_map->movement[1] = val;
			}

			else if(axis == 2) {
				inp_map->camera[0] = val;
			}
			else if(axis == 3) {
				inp_map->camera[1] = val;
			}

			break;

		case(XSDL_MOUSEWHEEL):
			cam_zoom(evt->wheel.y);
			break;

		case(XSDL_MOUSEMOTION):
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

V_API void game_update(void)
{
	vec3_t vel, forw, right;

	/* Rotate the camera */	
	cam_rot(inp_map->camera[0], inp_map->camera[1]);

	/* Set player-velocity */
	vec3_cpy(forw, camera->forward);
	forw[1] = 0.0;
	vec3_nrm(forw, forw);

	vec3_cpy(right, camera->right);
	right[1] = 0.0;
	vec3_nrm(right, right);

	vec3_scl(forw, inp_map->movement[1], forw);
	vec3_scl(right, inp_map->movement[0], right);

	vec3_add(forw, right, vel);

	vec3_cpy(objects->vel[core->obj], vel);

	obj_sys_update(1.0);

	/* Update the camera-position */
	cam_update();
}

V_API void game_render(void)
{
	wld_render();
	obj_sys_render();
}
