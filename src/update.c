#include "update.h"
#include "header.h"

#include <stdlib.h>

void game_start(void)
{
	uint32_t ts;

	/* Update core functions */
	core.proc_evt = &game_proc_evt;
	core.update = &game_update;
	core.render = &game_render;

	/* Setup timers */
	/* TODO: Consider if using last or next step is better */
	/* ts = ((net_gettime() / TICK_TIME) + 1) * TICK_TIME; */
	ts = (net_gettime() / TICK_TIME) * TICK_TIME;
	core.last_upd_ts = ts;
	core.last_ren_ts = ts;
	core.last_shr_ts = ts + SHARE_TIME;
	core.last_syn_ts = ts + SYNC_TIME;
}


void game_proc_evt(SDL_Event *evt)
{
	uint8_t axis;
	float val;
	float tmp;

	switch(evt->type) {
		case SDL_CONTROLLERAXISMOTION:
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

		case SDL_KEYDOWN:
			switch((int)evt->key.keysym.scancode) {
				case 4:  /* Pressed A */
					input.mov[0] = -1.0;
					break;
				case 7:  /* Pressed D */
					input.mov[0] = 1.0;
					break;
				case 22: /* Pressed S */
					input.mov[1] = 1.0;
					break;
				case 26: /* Pressed W */
					input.mov[1] = -1.0;
					break;
			}
			break;

		case SDL_KEYUP:
			switch((int)evt->key.keysym.scancode) {
				case 4:  /* Released A or D*/
				case 7:
					input.mov[0] = 0.0;
					break;
				case 22: /* Released W or S */
				case 26:
					input.mov[1] = 0.0;
					break;
			}
			break;


		case SDL_MOUSEWHEEL:
			cam_zoom(evt->wheel.y);
			break;

		case SDL_MOUSEMOTION:
			/* If left mouse button pressed */
			if(evt->motion.state == SDL_BUTTON_LMASK) {
				int x = -evt->motion.xrel;
				int y = -evt->motion.yrel;

				/* Rotate the camera */
				cam_rot(x, y);
			}
			break;

		case SDL_WINDOWEVENT:
			if(evt->window.event == SDL_WINDOWEVENT_RESIZED) {
				tmp = (float)window.win_w / (float)window.win_h;
				cam_proj_mat(45.0, tmp, 0.1, 1000.0);
			}
			break;
	}
}


static void game_proc_input(void)
{
	vec2_t forw;
	vec2_t right;
	vec2_t mov;
	short obj = core.obj;

	/*
	 * Control the camera.
	 */

	/* Rotate the camera */	
	cam_rot(input.cam[0], input.cam[1]);


	/* Get direction of the camera */
	vec2_set(right, camera.right[0], camera.right[2]);
	vec2_nrm(right, right);

	vec2_set(forw, camera.forward[0], camera.forward[2]);
	vec2_nrm(forw, forw);

	/* Combine input-direction and camera-direction */
	vec2_scl(right, input.mov[0], right);
	vec2_scl(forw, input.mov[1], forw);

	vec2_add(forw, right, mov);
	vec2_nrm(mov, mov);

	/* If the movement direction has changed */
	if(vec2_cmp(mov, input.mov_old) == 0) {
		short num = input.share.num;
		uint32_t ti = core.now_ts;
		uint8_t off = 0;

		if(num < INPUT_SLOTS) {
			if(input.share.timer == 0)
				input.share.timer = ti;
			else
				off = ti - input.share.timer;

			if(input.share.obj == 0)
				input.share.obj = objects.id[obj];

			/* Push new entry in share-buffer */	
			input.share.mask[num] = INP_M_MOV;
			if(input.share.off[num] == 0)
				input.share.off[num] = off;
			vec2_cpy(input.share.mov[num], mov);

			/* Increment share-buffer-number */
			input.share.num++;

			/* Remeber current state to check for changes */
			vec2_cpy(input.mov_old, mov);

			/* Add input to object input-buffer */
			obj_add_input(obj, 1, ti, mov, 0);
		}
	}
}


void game_update(void)
{
	static int c = 0;

	/* Get the current time */
	uint32_t now = net_gettime();
	int count = 0;
	char pck[512];
	int len = 1;
	int tmp;

	while((now - core.last_upd_ts) > TICK_TIME && count < MAX_UPDATE_NUM) {
		/* Update timer */
		core.now_ts = core.last_upd_ts;
		core.last_upd_ts += TICK_TIME;

		/* Process the game-input and update objects */
		game_proc_input();

		/* Process object-inputs */
		obj_sys_input();

		/* Update the objects in the object-table */
		obj_sys_update();

		/* Increment counter */
		count++;
	}
}

void game_render(void)
{
	uint32_t now = net_gettime();
	float interp = MIN(1.0, (float)((now - core.last_ren_ts) / TICK_TIME));

	/* Update the camera-position */
	cam_update();

	/* Render the world */
	wld_render(interp);

	/* Render the objects */
	obj_sys_render(interp);

	/* Update timer */
	core.last_ren_ts = now;
}
