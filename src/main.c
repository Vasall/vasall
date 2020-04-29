#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#include "core.h"
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
	vec3_t pos = {125.0, 0.0, 125.0};
	vec3_t dir = {1.0, 1.0, 1.0};
	vec3_nrm(dir, dir);

	srand(time(0));

	if(argc < 4) {
		printf("usage: %s <ipv6-addr> <port> <self>\n", argv[0]);
		return 0;
	}

	pad_printf("Initialize XSDL-subsystem");
	if(XSDL_Init(XSDL_INIT_EVERYTHING) < 0) {
		printf("failed!\n");
		printf("%s\n", SDL_GetError());
		return 0;
	}
	printf("done\n");

	pad_printf("Initialize core-wrapper");
	if(core_init(argc, argv) < 0) {
		printf("failed!\n");
		printf("%s\n", glo_get_err());
		goto err_cleanup_xsdl;
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
		goto err_cleanup_core;
	}
	printf("done\n");

	pad_printf("Initialize OpenGL");
	if(initGL() < 0) {
		printf("failed!\n");
		goto err_cleanup_window;
	}
	printf("done\n");

	pad_printf("Initialize UI-context");
	if((core->uicontext = XSDL_CreateUIContext(core->window)) == NULL) {
		printf("failed!\n");
		goto err_cleanup_gl;
	}
	core->uiroot = core->uicontext->root;
	printf("done\n");

	pad_printf("Init inputs");
	if(inp_init() < 0) {
		goto err_cleanup_ui;
	}
	printf("done\n");

	printf("Import models, textures and shaders:\n");
	if(core_load("../res") < 0) {
		goto err_cleanup_input;
	}

	printf("Import fonts:\n");
	if(loadResources() < 0) {
		goto err_cleanup_input;
	}

	pad_printf("Initialize UI");
	if(initUI() < 0) {
		printf("failed!\n");
		goto err_cleanup_input;
	}
	printf("done\n");

	pad_printf("Initialize camera");
	if(cam_init(45.0, 800.0 / 600.0, 0.1, 1000.0) < 0) {
		printf("failed!\n");
		goto err_cleanup_input;
	}
	cam_update();
	printf("done\n");

	pad_printf("Initialize world");
	if(wld_create() < 0) {
		printf("failed!\n");
		goto err_cleanup_camera;
	}
	printf("done\n");

	printf("\n");
	XSDL_ShowVersions();
	printf("OpenGL version: %s\n", glGetString(GL_VERSION));

	/* Add a demo-dummy */
	
	if((core->obj = obj_set(0, OBJ_M_ENTITY, pos, mdl_get("cube"), 
					NULL, 0)) < 0)
		goto err_cleanup_world;

	cam_trg_obj(core->obj);
	camera->dist = 10.0;
	cam_set_dir(dir);

	/*
	 * Mark the game as running and
	 * then proceed to jump into the
	 * main game-loop.
	 */
	core->running = 1;
	while(core->running) {
		handle_events();
		core_update();
		core_render();
	}

	printf("\n");

err_cleanup_world:
	pad_printf("Destroy world");
	wld_destroy();
	printf("done\n");

	pad_printf("Clear caches");
	XSDL_ClearFontCache();
	printf("done\n");

err_cleanup_camera:
	pad_printf("Destroy camera");
	cam_close();
	printf("done\n");

err_cleanup_input:
	pad_printf("Cleanup input");
	inp_close();
	printf("done\n");

err_cleanup_ui:
	pad_printf("Destroy UI-context");
	XSDL_DeleteUIContext(core->uicontext);
	printf("done\n");

err_cleanup_gl:
	pad_printf("Cleanup OpenGL");
	XSDL_GL_DeleteContext(core->glcontext);
	printf("done\n");

err_cleanup_window:
	pad_printf("Destroy window");
	XSDL_DestroyWindow(core->window);
	printf("done\n");

err_cleanup_core:
	pad_printf("Destroy the core-struct");
	core_close();
	printf("done\n");

err_cleanup_xsdl:
	pad_printf("Shutdown XSDL-subsystem");
	XSDL_Quit();
	printf("done\n");
	return 0;
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
		if(core->procevt)
			core->procevt(&event);
	}
}
