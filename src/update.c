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
	ts = (net_gettime() / TICK_TIME) * TICK_TIME;
	core.last_upd_ts = ts;
	core.last_ren_ts = ts;
	core.last_shr_ts = ts + SHARE_TIME;
	core.last_syn_ts = ts + SYNC_TIME;
}


void game_proc_evt(event_t *evt)
{
	uint8_t axis;
	float val;
	float tmp;
	vec3_t vtmp;

	vec2_t mov;
	vec3_t dir;

	uint32_t ts = net_gettime();

	switch(evt->type) {
		case SDL_KEYDOWN:
			inp_retrieve(INP_T_MOV, mov);

			switch((int)evt->key.keysym.scancode) {
				case 4:  /* A-Key */
					mov[0] = -1.0;
					break;
				case 7:  /* D-Key */
					mov[0] = 1.0;
					break;
				case 22: /* S-Key */
					mov[1] = -1.0;
					break;
				case 26: /* W-Key */
					mov[1] = 1.0;
					break;

				case 23: /* T-Key */
					cam_tgl_mode();
					break;
			}

			inp_change(INP_T_MOV, ts, mov);
			break;

		case SDL_KEYUP:
			inp_retrieve(INP_T_MOV, mov);

			switch((int)evt->key.keysym.scancode) {
				case 4:  /* A-Key */
				case 7:  /* D-Key */
					mov[0] = 0.0;
					break;
				case 22: /* S-Key */
				case 26: /* W-Key */
					mov[1] = 0.0;
					break;
			}

			inp_change(INP_T_MOV, ts, mov);
			break;


		case SDL_MOUSEWHEEL:
			cam_zoom(evt->wheel.y);
			break;

		case SDL_MOUSEMOTION:
			/* If left mouse button pressed */
			if(evt->motion.state == SDL_BUTTON_LMASK) {
				int x = evt->motion.xrel;
				int y = evt->motion.yrel;
				
				/* Rotate the camera */
				cam_rot(x, y);

				if(cam_get_mode() == CAM_MODE_FPV) {
					inp_change(INP_T_DIR, ts,
							camera.v_forward);
				}
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

	/* Adjust the direction of the camera to the input */
	cam_proc_input();

	/* Get direction of the camera */
	vec2_cpy(right, camera.v_right);
	vec2_nrm(right, right);

	vec2_cpy(forw, camera.v_forward);
	vec2_nrm(forw, forw);

	/* Combine input-direction and camera-direction */
	vec2_scl(right, -input.mov[0], right);
	vec2_scl(forw, input.mov[1], forw);

	vec2_add(forw, right, mov);
	vec2_nrm(mov, mov);

	/* If the movement direction has changed */
	if(vec2_cmp(mov, input.mov_old) == 0) {
		short num = input.share.num;
		uint32_t ti = core.now_ts;

		if(num < INPUT_SLOTS) {
			input.share.obj = objects.id[obj];

			/* Push new entry in share-buffer */	
			input.share.mask[num] = INP_M_MOV;
			input.share.ts[num] = ti;
			vec2_cpy(input.share.mov[num], mov);

			/* Increment share-buffer-number */
			input.share.num++;

			/* Remeber current state to check for changes */
			vec2_cpy(input.mov_old, mov);

			/* Add input to object input-buffer */
			obj_add_input(obj, 1, ti, mov, 0);
		}
	}

#if 0
	/*
	 * Reset cursor position.
	 * This will reset the cursor-position to the center to the screen. But
	 * this always bugs around. 
	 */
	SDL_WarpMouseInWindow(window.win, window.win_w / 2, window.win_h / 2);	
#endif
}


void game_update(void)
{
	/* Get the current time */
	uint32_t now = net_gettime();
	int count = 0;


	/*
	 * Process the inputs, push the local ones into the pipe and sort the
	 * entries.
	 */
	inp_update(now);


	/*
	 * Update the objects using the latest inputs and use
	 * movement-propagation to continue expected movement, until new inputs
	 * arrive.
	 */
	obj_sys_update(now);


	if(now >= core.last_shr_ts) {
		uint8_t con_flg = 0;
		char pck[512];
		int len = 1;
		uint16_t col = OBJ_A_ID | OBJ_A_POS | OBJ_A_VEL | OBJ_A_MOV;

		/* Update timestamp */
		core.last_shr_ts = now + SHARE_TIME;

		/* Only send if something has changed */
		if(input.share.num > 0) {
			/* Update content-flag */
			con_flg |= (1<<0);

			/* Collect all recent inputs */
			len += inp_col_share(pck + len);
		}

		if(now >= core.last_syn_ts) {
			int tmp;
			void *ptr;
			uint32_t id = objects.id[core.obj];

			/* Update content-flag */
			con_flg |= (1<<1);

			/* Add timestamp */
			memcpy(pck + len, &core.now_ts, 4);
			len += 4;

			/* Collect object data */
			tmp = obj_collect(col, &id, 1, &ptr, NULL);	

			/* Copy and then free object-data */
			memcpy(pck + len, ptr, tmp);
			free(ptr);

			len += tmp;

			/* Update timestamp */
			core.last_syn_ts = now + SYNC_TIME; 
		}

		if(con_flg != 0) {
			/* Copy content-flag */
			memcpy(pck, &con_flg, 1);

			/* Send packet */
			net_broadcast(HDR_OP_UPD, pck, len);
		}
	}


	/* Update the camera */
	cam_update();

	
	/* Reset local input-log */
	inp_reset_loc();

	/* Clear both input-pipes */
	inp_pipe_clear(INP_PIPE_IN);
	inp_pipe_clear(INP_PIPE_OUT);
}

void game_render(void)
{
	uint32_t now = net_gettime();
	float interp = (float)(now - core.last_ren_ts) / (float)TICK_TIME;

	/* Update timer */
	core.last_ren_ts = now;

	/* Interpolate the positions of the objects */
	obj_sys_prerender(interp);

	/* Update the camera-position */
	cam_update();

	/* Render the world */
	wld_render(interp);

	/* Render the objects */
	obj_sys_render();
}
