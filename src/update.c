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
	core.last_share = SDL_GetTicks();
}


void game_proc_evt(SDL_Event *evt)
{
	uint8_t axis;
	float val;

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
				case 0: input.mov[0] = val; break;
				case 1: input.mov[1] = val; break;
				case 2: input.cam[0] = val; break;
				case 3: input.cam[1] = val; break;
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
	vec3_t forw, right, mov;
	short obj = core.obj[0];

	/*
	 * Control the camera.
	 */

	/* Rotate the camera */	
	cam_rot(input.cam[0], input.cam[1]);

	/* Get direction of the camera */
	vec3_cpy(right, camera.right);
	right[1] = 0.0;
	vec3_nrm(right, right);

	vec3_cpy(forw, camera.forward);
	forw[1] = 0.0;
	vec3_nrm(forw, forw);

	/* Combine input-direction and camera-direction */
	vec3_scl(right, input.mov[0], right);
	vec3_scl(forw, input.mov[1], forw);

	vec3_add(forw, right, mov);
	vec3_nrm(mov, mov);

	/* If the movement direction has changed */
	if(vec2_cmp(mov, input.mov_old) == 0) {
		short num = input.share.num;
		uint32_t ti = SDL_GetTicks();
		uint8_t off = 0;

		if(input.share.timer == 0)
			input.share.timer = ti;
		else
			off = ti - input.share.timer;

		/* Push new entry in share-buffer */	
		input.share.mask[num] = SHARE_M_MOV;
		if(input.share.off[num] == 0)
			input.share.off[num] = off;
		input.share.obj[num] = objects.id[obj];
		vec2_set(input.share.mov[num], mov[0], mov[2]);

		/* Increment share-buffer-number */
		/* input.share.num++; */

		/* Remeber current state to check for changes */
		vec3_cpy(input.mov_old, mov);

		/* Copy the movement-direction into the object */
		vec3_cpy(objects.mov[obj], mov);
	}
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
		obj_sys_update(TICK_TIME_S);

		/* Update the camera-position */
		cam_update();

		core.last_update += TICK_TIME;
		count++;
	}

	now = SDL_GetTicks();
}

void game_render(void)
{
	uint32_t now = SDL_GetTicks();
	float interp = MIN(1.0, (float)((now - core.last_update) / TICK_TIME));

	/* Render the world */

	/* Render the objects */
	obj_sys_render(interp);
}
