#include "handle.h"
#include "../enud/enud.h"

void menu_procevt(ENUD_Event *evt)
{
	if(evt){}
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
				mouseMoved(core->camera, 
						evt->motion.xrel, 
						evt->motion.yrel);

				printf("Rotation: %f/%f/%f\n",
						core->camera->rot[0],
						core->camera->rot[1],
						core->camera->rot[2]);
			}
			break;
	}
}

void game_update(void)
{
}

void game_render(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* Transform according to camera */
	glRotatef(core->camera->rot[0], 1, 0, 0);
	glRotatef(core->camera->rot[1], 0, 1, 0);
	glTranslatef(-core->camera->pos[0], 
			-core->camera->pos[1], 
			-core->camera->pos[2]);


	/* Render the terrain */
	renderTerrain(core->world);
}
