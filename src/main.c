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
void handle_evts(void);
void update(void);
void render(void);

int main(int argc, char **argv)
{
	vec3_t pos = {125.0, 0.0, 125.0};
	vec3_t dir = {1.0, 1.0, 1.0};
	vec3_nrm(dir, dir);

	srand(time(0));

	if(argc < 3) {
		printf("usage: %s <ipv6-addr> <port>\n", argv[0]);
		return 0;
	}

	pad_printf("Initialize window");
	if(win_init() < 0) {
		printf("failed!\n");
		return 0;
	}
	printf("done\n");

	pad_printf("Initialize render-assets");
	if(rat_init() < 0) {
		printf("failed!\n");
		goto err_close_window;
	}
	printf("done\n");

	pad_printf("Initialize core-wrapper");
	if(core_init(argc, argv) < 0) {
		printf("failed!\n");
		printf("%s\n", glo_get_err());
		goto err_cleanup_rat;
	}
	printf("done\n");

	pad_printf("Init inputs");
	if(inp_init() < 0) {
		goto err_cleanup_core;
	}
	printf("done\n");

	printf("Import models, textures and shaders:\n");
	if(core_load("../res") < 0) {
		goto err_cleanup_input;
	}

	printf("Import fonts:\n");
	if(load_res() < 0) {
		goto err_cleanup_input;
	}

	pad_printf("Initialize UI");
	if(init_ui() < 0) {
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
		handle_evts();
		core_update();
		core_render();
	}

	printf("\n");

err_cleanup_world:
	pad_printf("Destroy world");
	wld_destroy();
	printf("done\n");

	pad_printf("Clear caches");
	printf("done\n");

err_cleanup_camera:
	pad_printf("Destroy camera");
	cam_close();
	printf("done\n");

err_cleanup_input:
	pad_printf("Cleanup input");
	inp_close();
	printf("done\n");

err_cleanup_core:
	pad_printf("Destroy the core-struct");
	core_close();
	printf("done\n");

err_cleanup_rat:
	pad_printf("Cleanup render-assets");
	rat_close();
	printf("done\n");

err_close_window:
	pad_printf("Close window");
	win_close();
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
 * evts. Note that the custom
 * evt-callback-function will be
 * skipped if the user interacts with
 * the userinterface.
 */
void handle_evts(void)
{
	XSDL_Event evt;

	while(XSDL_PollEvent(&evt)) {
		if(evt.type == XSDL_QUIT) {
			core->running = 0;
			return;
		}

		if(evt.type == XSDL_KEYDOWN && 
				evt.key.keysym.scancode == 20 &&
				evt.key.keysym.mod & KMOD_CTRL) {
			core->running = 0;
			return;
		}


		/* Process interactions with the UI */
		if(XSDL_ProcEvent(window->ui_ctx, &evt) > -1) {
			/* Skip, as the user interacted with the UI */
			continue;
		}

		if(evt.type == XSDL_WINDOWEVENT) {
			switch(evt.window.event) {
				case(XSDL_WINDOWEVENT_RESIZED):
					handle_resize(&evt);
					break;
			}
		}

		/* Run specified evt-handler */
		if(core->procevt)
			core->procevt(&evt);
	}
}
