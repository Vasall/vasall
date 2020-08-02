#include "core.h"
#include "object.h"
#include "camera.h"
#include "input.h"

#include <stdlib.h>


/* Redefine the global core-wrapper */
struct core_wrapper core; 


extern int core_init(void)
{
	core.running = 1;

	core.proc_evt = NULL;
	core.update = NULL;
	core.render = NULL;

	core.obj = -1;

	core.last_update = 0;
	core.last_render = 0;

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
			core.running = 0;
			return;
		}

		if(type == SDL_KEYDOWN && key == SDLK_q && (mod & KMOD_CTRL)) {
			core.running = 0;
			return;
		}

		if(win_proc_evt(&evt) > -1)
			continue;

		if(core.proc_evt)
			core.proc_evt(&evt);

#if 0
		if(evt.type == SDL_WINDOWEVENT) {
			switch(evt.window.event) {
				case(XSDL_WINDOWEVENT_RESIZED):
					handle_resize(&evt);
					break;
			}
		}
#endif
	}
}


extern void core_update(void)
{
	net_update();

	win_update();

	if(core.update)
		core.update();
}


extern void core_render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(core.render)
		core.render();

	win_render();

	SDL_GL_SwapWindow(window.win);
}
