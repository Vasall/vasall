/* Using SDL and standard IO */                                                                      
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#define ENUD_IMPLEMENTATION
#define ENUD_DEFINE_LIB
#define ENUD_DEFINE_ONCE
#include "../enud/enud.h"
#include "wrapper.h"
#include "setup.h"
#include "handle.h"

/* Prototypes */
void handle_events(void);
void update(void);
void render(void);

int main(int argc, char **argv)
{
	printf("Starting vasall-client.\n");

	if(init(argc, argv) < 0) {
		goto exit;
	}	

	core->running = 1;
	while(core->running) {
		handle_events();
		update();
		render();
	}


	/* Clear the font-cache */
	ENUD_ClearFontCache();

	/* Quit the ENUD-subsystem */
	ENUD_Quit();

exit:
	return(0);
}

/*
 * Process and handle the current 
 * events.
 */
void handle_events(void)
{
	ENUD_Event event;

	/* -------- EVENTS -------- */                                                       
	/* Process user-input */
	while(ENUD_PollEvent(&event)) {
		if(event.type == ENUD_QUIT) {
			core->running = 0;
			break;
		}

		/* Process interactions with the UI */
		if(ENUD_ProcEvent(core->uicontext, &event) > -1) {
			continue;
		}

		if(event.type == ENUD_WINDOWEVENT) {
			switch(event.window.event) {
				case(ENUD_WINDOWEVENT_RESIZED):
					handle_resize(&event);
					break;
			}
		}

		/* Run specified event-handler */
		if(core->procevt != NULL) {
			core->procevt(&event);
		}
	}
}

/*
 * Update the current game.
 */
void update(void)
{
	/* Run specified update-function */
	if(core->update != NULL) {
		core->update();
	}
}

/*
 * Render the game.
 */
void render(void)
{
	/* Clear the window */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Run specified render-function */
	if(core->render != NULL) {
		core->render();
	}

	/* Render the user-interface */
	ENUD_RenderPipe(core->uicontext);

	/* Render the pixel-buffer */
	ENUD_GL_SwapWindow(core->window);

}
