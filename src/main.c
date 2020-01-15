#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#define XSDL_IMPLEMENTATION
#define XSDL_DEFINE_LIB
#define XSDL_DEFINE_ONCE
#include "XSDL/xsdl.h"
#include "global.h"
#include "setup.h"
#include "handle.h"

#define PAD_LEN 30

void handle_events(void);
void update(void);
void render(void);

float f = 0.0;

int main(int argc, char **argv)
{
	int n;
	Vec3 pos = {0.0, 1.0, 0.0};

	if(argc) {/* Prevent warning for not using argc */}

	n = printf("Initialize XSDL-subsystem...");
	for(n = PAD_LEN - n; n >= 0; n--) printf(".");
	if(XSDL_Init(XSDL_INIT_EVERYTHING) < 0) {                                                    
		printf("failed!\n");
		printf("%s\n", SDL_GetError());
		goto exit;
	}
	printf("done\n");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	n = printf("Initialize core-wrapper...");
	for(n = PAD_LEN - n; n >= 0; n--) printf(".");
	if(gloInit() < 0) {
		printf("failed!\n");
		printf("%s\n", gloGetError());
		goto cleanup_enud;
	}
	printf("done\n");

	n = printf("Initialize window...");
	for(n = PAD_LEN - n; n >= 0; n--) printf(".");
	if((core->window = initWindow()) == NULL) {
		printf("failed!\n");
		printf("%s\n", XSDL_GetError());
		goto cleanup_core;
	}
	printf("done\n");

	n = printf("Initialize OpenGL...");
	for(n = PAD_LEN - n; n >= 0; n--) printf(".");
	if(initGL() < 0) {
		printf("failed!\n");
		goto cleanup_window;
	}
	printf("done\n");

	n = printf("Initialize UI-context...");
	for(n = PAD_LEN - n; n >= 0; n--) printf(".");
	if((core->uicontext = XSDL_CreateUIContext(core->window)) == NULL) {
		printf("failed!\n");
		goto cleanup_gl;
	}
	core->uiroot = core->uicontext->root;
	printf("done\n");

	core->bindir = XSDL_GetBinDir(argv[0]);

	printf("Load resources:\n");
	if(loadResources() < 0) {
		printf("failed!\n");
		goto cleanup_ui;
	}

	n = printf("Initialize UI...");
	for(n = PAD_LEN - n; n >= 0; n--) printf(".");
	if(initUI() < 0) {
		printf("failed!\n");
		goto cleanup_ui;
	}
	printf("done\n");

	n = printf("Initialize camera...");
	for(n = PAD_LEN - n; n >= 0; n--) printf(".");
	if((core->camera = camCreate(45.0, 800.0 / 600.0, 0.1, 1000.0)) == NULL) {
		printf("failed!\n");
		goto cleanup_ui;
	}
	camSetViewMat(core->camera);
	printf("done\n");

	n = printf("Initialize world...");
	for(n = PAD_LEN - n; n >= 0; n--) printf(".");
	if((core->world = wldCreate()) == NULL) {
		printf("failed!\n");
		goto cleanup_camera;
	}
	printf("done\n");

	printf("\n");	
	XSDL_ShowVersions();
	printf("OpenGL version: %s\n", glGetString(GL_VERSION));

	objPrint(core->player);
	objSetModel(core->player, mdlRedCube());

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

	XSDL_ClearFontCache();

cleanup_camera:
	camDestroy(core->camera);

cleanup_ui:
	XSDL_DeleteUIContext(core->uicontext);

cleanup_gl:
	XSDL_GL_DeleteContext(core->glcontext);

cleanup_window:
	XSDL_DestroyWindow(core->window);

cleanup_core:
	gloClose();

cleanup_enud:
	XSDL_Quit();

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
	XSDL_Event event;

	while(XSDL_PollEvent(&event)) {
		if(event.type == XSDL_QUIT) {
			core->running = 0;
			break;
		}

		/* Process interactions with the UI */
		if(XSDL_ProcEvent(core->uicontext, &event) > -1) {
			/* Skip, as the user interacted with the UI */
			continue;
		}

		if(event.type == XSDL_WINDOWEVENT) {
			switch(event.window.event) {
				case(XSDL_WINDOWEVENT_RESIZED):
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

	/* XSDL_RenderPipe(core->uicontext); */

	XSDL_GL_SwapWindow(core->window);
}
