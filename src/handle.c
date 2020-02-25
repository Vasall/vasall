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
void handle_resize(XSDL_Event *evt)
{
	double ratio;
	XSDL_UIContext *ctx = core->uicontext;

	if(evt){/* Prevent warning for not using evt */}

	printf("%d/%d\n", ctx->win_w, ctx->win_h);

	/* Update the viewport */
	glViewport(0, 0, ctx->win_w, ctx->win_h);

	/* Update projection matrix */
	ratio = (double)ctx->win_w / (double)ctx->win_h;
	camSetProjMat(45.0, ratio, 0.1, 1000.0);
}


void menu_procevt(XSDL_Event *evt)
{
	if(evt) {/* Prevent warning for not using evt */}
}

void menu_update(void)
{
}

void game_procevt(XSDL_Event *evt)
{
	int mod, mod_ctrl;
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
			camZoom(evt->wheel.y);
			break;

		case(XSDL_MOUSEMOTION):
			if(evt->motion.state == SDL_BUTTON_LMASK) {
				/* If left mouse button pressed */
				camRot(-evt->motion.xrel / 50.0,
						-evt->motion.yrel / 50.0);
			}
			break;

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
	}
}

void game_update(void)
{
	Vec3 vel, forw, right;

	/* Rotate the camera */	
	camRot(inp_map->camera[0], inp_map->camera[1]);

	/*printf("%f - %f\n", inp_map->movement[0], inp_map->movement[1]);*/

	/* Set player-velocity */
	vecCpy(forw, camera->forward);
	forw[1] = 0.0;
	vecNrm(forw, forw);

	vecCpy(right, camera->right);
	right[1] = 0.0;
	vecNrm(right, right);

	vecScl(forw, inp_map->movement[1], forw);
	vecScl(right, inp_map->movement[0], right);

	vecAdd(forw, right, vel);

	objSetVel(core->obj, vel);

	/* Update the object */
	objUpdate(core->obj, 1.0);

	/* Update the camera-position */
	camUpdate();
}

void game_render(void)
{
	/* Render the world */
	camUpdViewMat();
	wldRender();

	objRender(core->obj);
}
