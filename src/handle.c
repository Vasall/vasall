#include "handle.h"
#include "../enud/enud.h"

/*
 * This function is used as a
 * callback-function to handle
 * resizing the window.
 *
 * @evt: A pointer to the event
*/
void handle_resize(ENUD_Event *evt)
{
	int ratio;
	ENUD_UIContext *ctx = core->uicontext;	

	ratio = ctx->win_w / ctx->win_h;
	printf("%d\n", ratio);
	

	if(evt){/* Prevent warning for not using evt */}

	/* Update the viewport */
	/*glViewport(0, 0, ctx->win_w, ctx->win_h);*/

	/* Update projection matrix */
	/*glOrtho(-50 * ratio, 50 * ratio, -50, 50, -200, 200);*/
	/*glOrtho(-50 * ratio, 50 * ratio, -50, 50, -200, 200);	*/
	/*glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);*/
	/*gluPerspective(40, ratio, 0.1, 100);*/

}


void menu_procevt(ENUD_Event *evt)
{
	if(evt){/* Prevent warning for not using evt */}
}

void menu_update(void)
{
}

void game_procevt(ENUD_Event *evt)
{
	switch(evt->type) {
		case(ENUD_MOUSEMOTION):
			if(SDL_GetMouseState(NULL, NULL) & 
					SDL_BUTTON(SDL_BUTTON_LEFT)) {
				camMouseMoved(core->camera, 
						evt->motion.xrel, 
						evt->motion.yrel);

				printf("Rotation: %f/%f\n",
						core->camera->rot.x,
						core->camera->rot.y);
			}
			break;

		case(ENUD_MOUSEWHEEL):
			camZoom(core->camera, evt->wheel.y);
			break;
	}
}

void game_update(void)
{
}

void game_render(void)
{
	/* Render the terrain */
	renderTerrain(core->world);
}
