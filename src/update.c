#include "update.h"

/*
 * Process input-events on the menuscreen.
 * This function is a screen specific
 * callback function used to handle
 * interactions happening on the menuscreen.
 *
 * @evt: Pointer to the event-struct
 */
void menu_procevt(ENUD_Event *evt)
{

}

/*
 * Update the menuscreen.
 * This function is a screen specific
 * callback function used to handle 
 * updates for the menuscreen.
 */
void menu_update()
{

}

int8_t use_drag = 0;
float old_cam_x, old_cam_y;
int old_mouse_x, old_mouse_y;
int mouse_x, mouse_y;
float del_mouse_x, del_mouse_y;
float tile_size;
float del_wheel;

#ifdef TODO_DEBUF
void limit_camera()
{
	float min_x, min_y, max_x, max_y;
	float f = g_camera.tilesz * g_camera.zoom;

	min_x = ((g_context->win_w / 2) / f);
	min_y = ((g_context->win_h / 2) / f);

	if(g_camera->x < min_x) g_camera->x = min_x;

}
#endif


/*
 * Process input-events on the gamescreen.
 * This function is a screen specific
 * callback function used to handle
 * interactions happening on the gamescreen.
 *
 * @evt: Pointer to the event-struct
 */
void game_procevt(ENUD_Event *evt)
{
	switch(evt->type) {
		case(ENUD_MOUSEBUTTONDOWN):
			if(evt->button.button == ENUD_BUTTON_RIGHT) {
				use_drag = 1;

				old_cam_x = g_camera.x;
				old_cam_y = g_camera.y;

				ENUD_GetMouseState(&old_mouse_x, &old_mouse_y);
			}

			break;

		case(ENUD_MOUSEBUTTONUP):
			if(evt->button.button == ENUD_BUTTON_RIGHT) {
				use_drag = 0;
			}
			break;

		case(ENUD_MOUSEMOTION):
			if(use_drag) {
				ENUD_GetMouseState(&mouse_x, &mouse_y);

				tile_size = g_camera.zoom * g_camera.tilesz;
				del_mouse_x = ((old_mouse_x - mouse_x) / tile_size);
				del_mouse_y = ((old_mouse_y - mouse_y) / tile_size);
				g_camera.x = old_cam_x + del_mouse_x;
				g_camera.y = old_cam_y + del_mouse_y;
			}
			break;

		case(ENUD_MOUSEWHEEL):
			del_wheel = ((float)evt->wheel.y / 4);
			g_camera.zoom += del_wheel;
			if(g_camera.zoom < 0.25) g_camera.zoom = 0.25; 
			break;
	}
}

/*
 * Update the gamescreen.
 * This function is a screen specific
 * callback function used to handle 
 * updates for the gamescreen.
 */
void game_update()
{
	wld_render(g_renderer, &g_camera, g_world);

	ENUD_RenderSprite(g_renderer, 
		g_context, &g_camera,
		ENUD_Sprites[1], 
		0, 
		20 + 0.5,
		20 + 0.5);

}
