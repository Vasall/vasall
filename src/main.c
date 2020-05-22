#include <stdio.h>
#include <stdlib.h>

#include "filesystem.h"
#include "core.h"
#include "setup.h"

void handle_events(void);

int main(int argc, char **argv)
{
	vec3_t dir = {1.0, -1.0, 1.0};
	vec3_t pos = {125.0, 0.0, 125.0};
	short tmp;

	if(argc || argv) {/* Prevent warnings for not using parameters */}

	srand(time(0)); 

	/* Initialize the network-system */
	if(net_init() < 0)
		return 0;

	printf("Internal: %s\n", net_str_addr6(&network.int_addr));
	printf("External: %s\n", net_str_addr6(&network.ext_addr));

	/* Initialize the sdl-subsystem */
	if(sdl_init() < 0)
		goto err_close_net;

	/* Initialize asset-table(shaders, textures, fonts) */
	if(ast_init() < 0)
		goto err_close_sdl;

	/* Initialize the model-table */
	if(mdl_init() < 0)
		goto err_free_ast;

	/* Initialize the window and opengl-context */
	if(win_init() < 0)
		goto err_close_mdl;

	/* Initialize the input-device-table and input-table */
	if(inp_init() < 0)
		goto err_close_window;

	/* Load the basic resources (fonts, shaders, textures, models) */
	if(load_resources() < 0)
		goto err_close_input;
	
	/* Initialize the camera */
	if(cam_init(45.0, 800.0 / 600.0, 0.1, 1000.0) < 0)
		goto err_close_input;

	/* Initialize the world and world-objects */
	if(wld_init() < 0)
		goto err_close_camera;

	/* Initialize the object-table */
	if(obj_init() < 0)
		goto err_close_world;

	/* Load the user-interface-nodes */
	if(load_ui() < 0)
		goto err_close_obj;

	/* Initialize the core */
	if(core_init() < 0)
		goto err_close_obj;

	/* Add player-object */
	tmp = mdl_get("plr");
	if((core.obj = obj_set(0, OBJ_M_ENTITY, pos, tmp, NULL, 0)) < 0)
		goto err_close_obj;

	cam_trg_obj(core.obj);
	camera.dist = 10.0;
	cam_set_dir(dir);

	while(core.running) {
		core_proc_evt();
		core_update();
		core_render();
	}

err_close_obj:
	obj_close();

err_close_world:
	wld_close();

err_close_camera:
	cam_close();

err_close_input:
	inp_close();

err_close_window:
	win_close();

err_close_mdl:
	mdl_close();

err_free_ast:
	ast_close();

err_close_sdl:
	sdl_close();

err_close_net:
	net_close();
	return -1;
}
