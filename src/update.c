#include "update.h"

#include <stdlib.h>

void game_start(void)
{
	/* Update core functions */
	core.proc_evt = &game_proc_evt;
	core.update = &game_update;
	core.render = &game_render;

	/* Setup timers */
	core.last_update = SDL_GetTicks();
	core.last_render = SDL_GetTicks();
}


void game_proc_evt(SDL_Event *evt)
{
	int mod_ctrl;
	uint8_t axis;
	float val;

	if(mod_ctrl) {/* Prevent warning for not using mod_ctrl */}

	switch(evt->type) {
		case(SDL_CONTROLLERAXISMOTION):
			axis = evt->caxis.axis;
			val = evt->caxis.value;

			/* Convert range to [-1; 1]*/
			if(val < 0.0) val = val / 32768.0;
			else val = val / 32767.0;

			/*
			 * Set the value of the dedicated input-buffer. Note
			 * that the first element is for vertical data and the
			 * second element for horizontal data.
			 */
			switch(axis) {
				case 0: input.movement[0] = val; break;
				case 1: input.movement[1] = val; break;
				case 2: input.camera[0] = val; break;
				case 3: input.camera[1] = val; break;
			}
			break;

		case(SDL_MOUSEWHEEL):
			cam_zoom(evt->wheel.y);
			break;

		case(SDL_MOUSEMOTION):
			/* If left mouse button pressed */
			if(evt->motion.state == SDL_BUTTON_LMASK) {
				int x = -evt->motion.xrel;
				int y = -evt->motion.yrel;

				/* Rotate the camera */
				cam_rot(x, y);
			}
			break;
	}
}


static void game_proc_input(void)
{
	vec3_t acl, forw, right, fac;

	/*
	 * Control the camera.
	 */

	/* Rotate the camera */	
	cam_rot(input.camera[0], input.camera[1]);

	/*
	 * Control the speed and direction of the character.
	 */

	/* Get direction of the camera */
	vec3_cpy(right, camera.right);
	right[1] = 0.0;
	vec3_nrm(right, right);

	vec3_cpy(forw, camera.forward);
	forw[1] = 0.0;
	vec3_nrm(forw, forw);

	/* Combine input-direction and camera-direction */
	vec3_scl(right, input.movement[0], right);
	vec3_scl(forw, input.movement[1], forw);

	vec3_add(forw, right, fac);
	vec3_nrm(fac, fac);

	/* Calculate the actual acceleration for the player */
	vec3_scl(fac, 5.0, acl);

	/* Copy the acceleration to the player-object */
	vec3_cpy(objects.vel[core.obj], acl);
}


void game_update(void)
{
	/* Get the current time */
	uint32_t now = SDL_GetTicks();
	int count = 0;

	while(now - core.last_update > TICK_TIME && count < MAX_UPDATE_NUM) {
		/* Process the game-input and update objects */
		game_proc_input();

		/* Update the objects in the object-table */
		obj_sys_update(TICK_TIME / 1000.0);

		/* Update the camera-position */
		cam_update();

		core.last_update += TICK_TIME;
		count++;
	}
}

void game_render(void)
{
	uint32_t now = SDL_GetTicks();
	float interp = MIN(1.0, (float)((now - core.last_update) / TICK_TIME));

	/* Render the world */
	wld_render(interp);

	/* Render the objects */
	obj_sys_render(interp);
}
