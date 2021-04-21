#include "core.h"
#include "object.h"
#include "camera.h"
#include "input.h"

#include <stdlib.h>


/* Redefine the global core-wrapper */
struct core_wrapper g_core; 


extern int core_init(void)
{
	g_core.running = 1;

	g_core.proc_evt = NULL;
	g_core.update = NULL;
	g_core.render = NULL;

	g_core.obj = -1;

	g_core.last_upd_ts = 0;
	g_core.last_ren_ts = 0;
	g_core.last_shr_ts = 0;
	g_core.last_syn_ts = 0;

	return 0;
}


extern void core_proc_evt(void)
{
	SDL_Event evt;
	uint32_t type;
	uint32_t mod;
	SDL_Keycode key;

	while(SDL_PollEvent(&evt)) {
		type = evt.type;
		mod = evt.key.keysym.mod;
		key = evt.key.keysym.sym;
		
		if(type == SDL_QUIT) {
			g_core.running = 0;
			return;
		}

		if(type == SDL_KEYDOWN && key == SDLK_q && (mod & KMOD_CTRL)) {
			g_core.running = 0;
			return;
		}

		if(win_proc_evt(&evt) > -1) {
			continue;
		}

		if(g_core.proc_evt) {
			g_core.proc_evt(&evt);
		}
	}
}


extern void core_update(void)
{
	net_update();

	win_update();

	if(g_core.update) {
		g_core.update();
	}
}


extern void core_render(void)
{
	ren_start();

	if(g_core.render)
		g_core.render();

	win_render();

	ren_end(g_win.win);
}
