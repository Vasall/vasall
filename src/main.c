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
#include "global.h"
#include "setup.h"
#include "handle.h"

void handle_events(void);
void update(void);
void render(void);

float f = 0.0;

int main(int argc, char **argv)
{
	if(argc){/* Prevent warning for not using argc */}

	printf("Initialize ENUD-subsystem\n");
	if(ENUD_Init(ENUD_INIT_EVERYTHING) < 0) {                                                    
		printf("[!] ENUD could not initialize! (%s)\n", 
				SDL_GetError());
		goto exit;
	}
	ENUD_ShowVersions();

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	printf("Initialize core-wrapper\n");
	if((core = gloCreate()) == NULL) {
		printf("Failed to initialize core-wrapper.\n");
		goto cleanup_enud;
	}

	printf("Initialize window\n");
	if((core->window = initWindow()) == NULL) {
		printf("[!] Failed to create window! (%s)\n", 
				ENUD_GetError());
		goto cleanup_core;
	}

	printf("Initialize OpenGL\n");
	if(initGL() < 0) {
		printf("[!] Failed to initialize OpenGL.\n");
		goto cleanup_window;
	}
	printf("OpenGL version: %s\n", glGetString(GL_VERSION));

	printf("Initialize UI-context\n");
	if((core->uicontext = ENUD_CreateUIContext(core->window)) == NULL) {
		printf("[!] Failed to setup ui-context.\n");
		goto cleanup_gl;
	}
	core->uiroot = core->uicontext->root;

	core->bindir = ENUD_GetBinDir(argv[0]);

	printf("Load resources\n");
	if(loadResources() < 0) {
		printf("[!] Failed to load resources.\n");
		goto cleanup_ui;
	}

	printf("Initialize userinterface\n");
	if(initUI() < 0) {
		printf("[!] Failed to setup ui.\n");
		goto cleanup_ui;
	}

	printf("Initialize camera\n");
	if((core->camera = camCreate(45.0, 800.0 / 600.0, 0.1, 1000.0)) == NULL) {
		printf("[!] Failed ot setup camera.\n");
		goto cleanup_ui;
	}
	camSetViewMat(core->camera, 0.0, 0.0, 0.0, 100.0, 100.0, 100.0);

	printf("Initialize world-container\n");
	if((core->world = wldCreate()) == NULL) {
		printf("[!] Failed to initialize world.\n");
		goto cleanup_camera;
	}

	try_login(NULL, NULL);	

	/* 
	 * Mark the game as running and
	 * then proceed to jump into the
	 * main game-loop. 
	*/
	core->running = 1;
	while(core->running) {
		handle_events();
		update();
		render();
	}


	wldDestroy(core->world);

	ENUD_ClearFontCache();

cleanup_camera:
	camDestroy(core->camera);

cleanup_ui:
	ENUD_DeleteUIContext(core->uicontext);

cleanup_gl:
	ENUD_GL_DeleteContext(core->glcontext);

cleanup_window:
	ENUD_DestroyWindow(core->window);

cleanup_core:
	gloDestroy(core);

cleanup_enud:
	ENUD_Quit();

exit:
	return(0);
}

/*
 * Process and handle the current 
 * events. Note that the custom
 * event-callback-function will be
 * skipped if the user interacts with
 * the userinterface.
 */
void handle_events(void)
{
	ENUD_Event event;
	int mod_ctrl, mod_shift;

	while(ENUD_PollEvent(&event)) {
		if(event.type == ENUD_QUIT) {
			core->running = 0;
			break;
		}

		if(event.type == SDL_KEYDOWN) {
			if(event.key.keysym.scancode == 20 &&
					event.key.keysym.mod & KMOD_CTRL) {
				core->running = 0;
				break;
			}

			mod_ctrl = event.key.keysym.mod &
				(KMOD_LCTRL | KMOD_RCTRL);
			mod_shift = event.key.keysym.mod & 
				(KMOD_LSHIFT | KMOD_RSHIFT);
			switch(event.key.keysym.sym) {
				case SDLK_w:
					camMovDir(core->camera, FORWARD, mod_shift);
					break;
				case SDLK_s:
					camMovDir(core->camera, BACK, mod_shift);
					break;
				case SDLK_a:
					camMovDir(core->camera, LEFT, mod_shift);
					break;
				case SDLK_d:
					camMovDir(core->camera, RIGHT, mod_shift);
					break;
				default:
					break;
			}
		}

		/* Process interactions with the UI */
		if(ENUD_ProcEvent(core->uicontext, &event) > -1) {
			/* Skip, as the user interacted with the UI */
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
 * Update the current game and execute
 * the update-callback-function.
 */
void update(void)
{
	/* Run specified update-function */
	if(core->update != NULL) {
		core->update();
	}

	/*mdlSetRot(core->world->model, 0.0, f, 0.0);
	f += 0.1;*/
}

/*
 * Render the game-scene and the 
 * user-interface and then execute the
 * render-callback-function.
 */
void render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Run current render-function */
	if(core->render != NULL) {
		core->render();
	}

	/* ENUD_RenderPipe(core->uicontext); */

	ENUD_GL_SwapWindow(core->window);
}
