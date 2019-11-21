#include "update.h"

void menu_procevt(ENUD_Event *evt)
{

}

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

void game_procevt(ENUD_Event *evt)
{
	switch(evt->type) {
		case(ENUD_MOUSEBUTTONDOWN):
			if(evt->button.button == ENUD_BUTTON_RIGHT) {
				use_drag = 1;

				old_cam_x = g_camera.pos.x;
				old_cam_y = g_camera.pos.y;

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
				g_camera.pos.x = old_cam_x + del_mouse_x;
				g_camera.pos.y = old_cam_y + del_mouse_y;
			}
			break;

		case(ENUD_MOUSEWHEEL):
			del_wheel = ((float)evt->wheel.y / 4);
			g_camera.zoom += del_wheel;
			if(g_camera.zoom < 0.25) g_camera.zoom = 0.25; 
			break;
	}
}

void game_update()
{
	vsRect building = {10.0, 10.0, 2.0, 3.0};
       	int x, y;
	int w, h;

	cam_getpos(&g_camera, building.x, building.y, &x, &y);
	cam_convun(&g_camera, building.w, building.h, &w, &h);

	ENUD_Rect rend = {x, y, w, h};

	ENUD_RenderCopy(g_renderer, ENUD_Images[1], NULL, &rend);

}
