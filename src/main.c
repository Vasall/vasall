#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#include "global.h"
#include "setup.h"
#include "handle.h"
#include "input.h"

#define PAD_LEN 30

void pad_printf(char *str);
void handle_events(void);
void update(void);
void render(void);

int main(int argc, char **argv)
{
	Vec3 pos = {125.0, 0.0, 125.0};
	Vec3 dir = {1.0, 1.0, 1.0};

	vecNrm(dir, dir);

	/* Update the rand-seed */
	srand(time(0));

	pad_printf("Initialize XSDL-subsystem");
	if(XSDL_Init(XSDL_INIT_EVERYTHING) < 0) {
		printf("failed!\n");
		printf("%s\n", SDL_GetError());
		goto exit;
	}
	printf("done\n");

	pad_printf("Initialize core-wrapper");
	if(gloInit(argc, argv) < 0) {
		printf("failed!\n");
		printf("%s\n", gloGetError());
		goto cleanup_enud;
	}
	printf("done\n");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_PROFILE_CORE);

	pad_printf("Initialize window");
	if((core->window = initWindow()) == NULL) {
		printf("failed!\n");
		printf("%s\n", XSDL_GetError());
		goto cleanup_core;
	}
	printf("done\n");

	pad_printf("Initialize OpenGL");
	if(initGL() < 0) {
		printf("failed!\n");
		goto cleanup_window;
	}
	printf("done\n");

	pad_printf("Initialize UI-context");
	if((core->uicontext = XSDL_CreateUIContext(core->window)) == NULL) {
		printf("failed!\n");
		goto cleanup_gl;
	}
	core->uiroot = core->uicontext->root;
	printf("done\n");

	pad_printf("Init inputs");
	if(inpInit() < 0) {
		goto cleanup_ui;
	}
	printf("done\n");

	printf("Import models, textures and shaders:\n");
	if(gloLoad("../res") < 0) {
		goto cleanup_ui;
	}

	printf("Import fonts:\n");
	if(loadResources() < 0) {
		goto cleanup_ui;
	}

	pad_printf("Initialize UI");
	if(initUI() < 0) {
		printf("failed!\n");
		goto cleanup_ui;
	}
	printf("done\n");

	pad_printf("Initialize camera");
	if(camCreate(45.0, 800.0 / 600.0, 0.1, 1000.0) < 0) {
		printf("failed!\n");
		goto cleanup_ui;
	}
	camUpdViewMat();
	printf("done\n");

	pad_printf("Initialize world");
	if(wldCreate() < 0) {
		printf("failed!\n");
		goto cleanup_camera;
	}
	printf("done\n");

	printf("\n");
	XSDL_ShowVersions();
	printf("OpenGL version: %s\n", glGetString(GL_VERSION));

	/* Add a demo-dummy */
	objSet("demo", "cube", pos);
	core->obj = objGet("demo");
	if(core->obj == NULL) goto cleanup_world;

	camTargetObj(core->obj);

	camera->dist = 10.0;
	camSetDir(dir);

	XSDL_ShowNodes(core->uicontext->root);

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

	printf("\n");

cleanup_world:
	pad_printf("Destroy world");
	wldDestroy();
	printf("done\n");

	pad_printf("Clear caches");
	XSDL_ClearFontCache();
	printf("done\n");

cleanup_camera:
	pad_printf("Destroy camera");
	camDestroy();
	printf("done\n");

cleanup_ui:
	pad_printf("Destroy UI-context");
	XSDL_DeleteUIContext(core->uicontext);
	printf("done\n");

cleanup_gl:
	pad_printf("Cleanup OpenGL");
	XSDL_GL_DeleteContext(core->glcontext);
	printf("done\n");

cleanup_window:
	pad_printf("Destroy window");
	XSDL_DestroyWindow(core->window);
	printf("done\n");

cleanup_core:
	pad_printf("Destroy the core-struct");
	gloClose();
	printf("done\n");

cleanup_enud:
	pad_printf("Shutdown XSDL-subsystem");
	XSDL_Quit();
	printf("done\n");

exit:
	return(0);
}

/*
 * Print a padded message in the terminal
 * and set the rest to dots.
 *
 * @str: The message to print in the console
 */
void pad_printf(char *str)
{
	int n = printf("%s", str);
	for(n = PAD_LEN - n; n >= 0; n--) printf(".");
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
			return;
		}

		if(event.type == XSDL_KEYDOWN && 
				event.key.keysym.scancode == 20 &&
				event.key.keysym.mod & KMOD_CTRL) {
			core->running = 0;
			return;
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
	XSDL_UpdateUIContext(core->uicontext);

	/* Run specified update-function */
	if(core->update != NULL) core->update();
}

/*
 * Render the game-scene and the
 * user-interface and then execute the
 * render-callback-function.
 */
void render(void)
{
	/* Clear the screen */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Run current render-function */
	if(core->render != NULL) core->render();

	/* Render the current userinterface */
	XSDL_Render(core->uicontext);

	/* Render the buffer on the screen */
	XSDL_GL_SwapWindow(core->window);
}
