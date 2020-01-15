#include "handle.h"
#include "XSDL/xsdl.h"

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
	camSetProjMat(core->camera, 45.0, ratio, 0.1, 1000.0);
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
	int mod, mod_ctrl, mod_shift;

	if(mod_ctrl) {/* Prevent warning for not using mod_ctrl */}

	switch(evt->type) {
		case(XSDL_MOUSEWHEEL):
			camZoom(core->camera, evt->wheel.y);
			break;

		case(XSDL_KEYDOWN):
			mod = evt->key.keysym.mod;
			if(evt->key.keysym.scancode == 20 && mod & KMOD_CTRL) {
				core->running = 0;
				break;
			}

			mod_ctrl = mod & (KMOD_LCTRL | KMOD_RCTRL);
			mod_shift = mod & (KMOD_LSHIFT | KMOD_RSHIFT);
			switch(evt->key.keysym.sym) {
				case(SDLK_w):
					camMovDir(core->camera, FORWARD);
					break;
				case(XSDLK_s):
					camMovDir(core->camera, BACK);
					break;
				case(XSDLK_a):
					camMovDir(core->camera, LEFT);
					break;
				case(XSDLK_d):
					camMovDir(core->camera, RIGHT);
					break;
				case(XSDLK_q):
					camZoom(core->camera, 1);
					break;
				case(XSDLK_e):
					camZoom(core->camera, -1);
					break;
				case(XSDLK_UP):
					camRot(core->camera, 0.0, 0.1);
					break;
				case(XSDLK_DOWN):
					camRot(core->camera, 0.0, -0.1);
					break;
				case(XSDLK_RIGHT):
					camRot(core->camera, 0.1, 0.0);
					break;
				case(XSDLK_LEFT):
					camRot(core->camera, -0.1, 0.0);
					break;
			}
			break;
	}
}

void game_update(void)
{

}

void game_render(void)
{
	/* Render the world */
	camUpdPos(core->camera);
	wldRender(core->world);

	/* Render the player */
	objRender(core->player);
}
