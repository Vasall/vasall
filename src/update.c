#include "update.h"
#include "net_header.h"

#include <stdlib.h>

void game_start(void)
{
	uint32_t ts;

	/* Update core functions */
	g_core.proc_evt = &game_proc_evt;
	g_core.update = &game_update;
	g_core.render = &game_render;

	/* Setup timers */
	ts = (net_gettime() / TICK_TIME) * TICK_TIME;
	g_core.last_upd_ts = ts;
	g_core.last_ren_ts = ts;
	g_core.last_shr_ts = ts + SHARE_TIME;
	g_core.last_syn_ts = ts + SYNC_TIME;
}


void game_proc_evt(event_t *evt)
{
	float tmp;

	vec2_t mov;

	uint32_t ts = net_gettime();

	switch(evt->type) {
		case SDL_KEYDOWN:
			inp_retrieve(INP_M_MOV, mov);

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

			inp_change(INP_M_MOV, ts, mov);
			break;

		case SDL_KEYUP:
			inp_retrieve(INP_M_MOV, mov);

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

			inp_change(INP_M_MOV, ts, mov);
			break;


		case SDL_MOUSEWHEEL:
			cam_zoom(evt->wheel.y);
			break;

		case SDL_MOUSEMOTION:
			/* If left mouse button pressed */
			if(evt->motion.state == SDL_BUTTON_LMASK || 1) {
				int x = evt->motion.xrel;
				int y = evt->motion.yrel;
				
				/* Rotate the camera */
				cam_rot(x, y);

				if(cam_get_mode() != CAM_MODE_FPV)
					break;
				
				inp_change(INP_M_DIR, ts, g_cam.v_forward);
			}
			break;

		case SDL_WINDOWEVENT:
			if(evt->window.event == SDL_WINDOWEVENT_RESIZED) {
				tmp = (float)g_win.win_w / (float)g_win.win_h;
				cam_proj_mat(45.0, tmp, 0.1, 1000.0);
			}
			break;
	}
}


static void game_proc_input(void)
{
	/*
	 * Control the camera.
	 */

	/*
	 * Reset cursor position.
	 * This will reset the cursor-position to the center to the screen. But
	 * this always bugs around. 
	 */
	SDL_WarpMouseInWindow(g_win.win, g_win.win_w / 2, g_win.win_h / 2);	
}


void game_update(void)
{
	/* Get the current time */
	uint32_t now = net_gettime();

	inp_proc();

	/*
	 * Process the inputs, push the local ones into the pipe and sort the
	 * entries.
	 */
	inp_update();


	/*
	 * Update the objects using the latest inputs and use
	 * movement-propagation to continue expected movement, until new inputs
	 * arrive.
	 */
	obj_sys_update(now);


#if 0
	if(now >= g_core.last_shr_ts) {
		uint8_t con_flg = 0;
		char pck[512];
		int len = 1;
		uint16_t col = OBJ_A_ID | OBJ_A_POS | OBJ_A_VEL | OBJ_A_MOV;

		/* Update timestamp */
		g_core.last_shr_ts = now + SHARE_TIME;

		/* Only send if something has changed */
		if(g_inp.share.num > 0) {
			/* Update content-flag */
			con_flg |= (1<<0);

			/* Collect all recent inputs */
			len += inp_col_share(pck + len);
		}

		if(now >= g_core.last_syn_ts) {
			int tmp;
			void *ptr;
			uint32_t id = g_obj.id[g_core.obj];

			/* Update content-flag */
			con_flg |= (1<<1);

			/* Add timestamp */
			memcpy(pck + len, &g_core.now_ts, 4);
			len += 4;

			/* Collect object data */
			tmp = obj_collect(col, &id, 1, &ptr, NULL);	

			/* Copy and then free object-data */
			memcpy(pck + len, ptr, tmp);
			free(ptr);

			len += tmp;

			/* Update timestamp */
			g_core.last_syn_ts = now + SYNC_TIME; 
		}

		if(con_flg != 0) {
			/* Copy content-flag */
			memcpy(pck, &con_flg, 1);

			/* Send packet */
			net_broadcast(HDR_OP_UPD, pck, len);
		}
	}
#endif



	/* Update the camera */
	cam_update();


	/* Clear both input-pipes */
	inp_pipe_clear(INP_PIPE_IN);
	inp_pipe_clear(INP_PIPE_OUT);

	/* Reset the input-log */
	g_inp.log.latest_slot = -1;
	g_inp.log.latest_itr = -1;
}

void game_render(void)
{
	uint32_t now = net_gettime();
	float interp = (float)(now - g_core.last_ren_ts) / (float)TICK_TIME;

	/* Update timer */
	g_core.last_ren_ts = now;

	/* Interpolate the positions of the objects */
	obj_sys_prerender(interp);

	/* Update the camera-position */
	cam_update();

	/* Render the world */
	wld_render(interp);

	/* Render the objects */
	obj_sys_render();
}
