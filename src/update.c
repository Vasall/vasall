#include "update.h"

void menu_procevt(ENUD_Event *evt)
{
}

void menu_update()
{
}

int ox = 0, oy = 0;
int tile_size = 32;

void game_procevt(ENUD_Event *evt)
{

}

void game_update()
{
	ox = (ox + 2) % tile_size;
	oy = (oy + 2) % tile_size;

	ENUD_Color col = {0xff, 0xff, 0xff, 0xff};
	
	ENUD_SetRenderDrawColor(g_renderer, &col);
		
	int x, y;
	for(x = ox; x < g_context->win_w; x += tile_size) {
		ENUD_RenderDrawLine(g_renderer, 
				x, 0, x, g_context->win_h);

		for(y = oy; y < g_context->win_h; y += tile_size) {
			ENUD_RenderDrawLine(g_renderer, 
					0, y, g_context->win_w, y);
		}
	}
}
