#include "window.h"

#include <stdlib.h>

/* Redefine global window-wrapper */
struct win_wrapper g_win;

static int win_setup_shader(void)
{
	char *vars[2] = {"pos", "tex"};

	ren_create_shader("res/shaders/ui.vert", "res/shaders/ui.frag",
				&g_win.shader, 
				&g_win.pipeline,
				2, vars, 
				MDL_TYPE_DEFAULT);

	return 0;
}

static int win_load_cursors(void)
{
	g_win.cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	g_win.cursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	g_win.cursors[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	return 0;
}

extern int win_init(void)
{
	int win_flgs;
	struct ui_node *root = NULL;
	char mode;

#if USE_OPENGL
	win_flgs = SDL_WINDOW_OPENGL;
	mode = 1;

	if(!(g_win.win = SDL_CreateWindow("Vasall",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					WIN_W, WIN_H, win_flgs)))
		return -1;
#else
	win_flgs = SDL_WINDOW_VULKAN;
	mode = 0;

	if(!(g_win.win = SDL_CreateWindow("Vasall",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					WIN_W, WIN_H, win_flgs)))
		return -1;
#endif

	if(ren_init(g_win.win, mode) < 0) {
		printf("Fallback to OpenGL\n");

		SDL_DestroyWindow(g_win.win);
		win_flgs = SDL_WINDOW_OPENGL;
		mode = 1;

		if(!(g_win.win = SDL_CreateWindow("Vasall",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					WIN_W, WIN_H, win_flgs)))
			return -1;
		
		if(ren_init(g_win.win, mode) < 0)
			goto err_close_window;
	}

	if(win_setup_shader() < 0)
		goto err_destroy_render;

	g_win.hover = NULL;
	g_win.active = NULL;

	g_win.win_w = WIN_W;
	g_win.win_h = WIN_H;

	if(!(g_win.root = ui_add(UI_WRAPPER, NULL, NULL, "root")))
		goto err_delete_shader;
		
	win_build_pipe();
	
	if(win_load_cursors() < 0)
		goto err_delete_root;

	return 0;

err_delete_root:
	ui_remv(root);

err_delete_shader:
	ren_destroy_shader(g_win.shader, g_win.pipeline);

err_destroy_render:
	ren_destroy();

err_close_window:
	SDL_DestroyWindow(g_win.win);
	g_win.win = NULL;
	return -1;
}

extern void win_close(void)
{
	ui_remv(g_win.root);
	ren_destroy_shader(g_win.shader, g_win.pipeline);
	ren_destroy();
	SDL_DestroyWindow(g_win.win);
}

extern void win_update(void)
{
	if(g_win.active) {
		if(g_win.active->events.onactive){
			g_win.active->events.onactive(g_win.active, NULL);
		}
	}
}

extern void win_render(void)
{
	short i;

	for(i = 0; i < g_win.pipe_num; i++) {
		ui_render(g_win.pipe[i]);
	}
}

static int win_onmousemotion(event_t *evt)
{
	int2_t pos;
	ui_node_fnc fnc;

	if(evt){/*Prevent warning from unused variable <evt>*/}

	SDL_GetMouseState(&pos[0], &pos[1]);
	g_win.hover = win_check_hover(g_win.root, pos);

	/* Hovering over element */
	if(g_win.hover != NULL) {
		struct ui_node *hovered = g_win.hover;

		/* Use flag-cursor */
		SDL_SetCursor(g_win.cursors[(int)hovered->flags.cur]);

		if((fnc = hovered->events.hover) != NULL)
			fnc(g_win.hover, NULL);

		return 1;
	}

	SDL_SetCursor(g_win.cursors[0]);
	return -1;
}

static int win_onmousebuttondown(event_t *evt)
{
	ui_node *hov = g_win.hover;
	ui_node *act = g_win.active;

	switch(evt->button.button) {
		case(1):
			if(hov == NULL && act != NULL) {
				win_unfocus_node();
				return 1;
			}
			else if(hov != NULL && hov->flags.enfoc == 1) {
				if(act == NULL) {
					win_focus_node(hov);
				}
				else if(strcmp(act->id, hov->id) != 0) {
					win_unfocus_node();
					win_focus_node(hov);
				}
			}

			if(hov != NULL && hov->events.mousedown != NULL) {
				hov->events.mousedown(hov, evt);
			}

			return 1;

		case(3):
			break;
	}

	return -1;
}

static int win_oncontrolleradded(event_t *evt)
{
	ctr_add_device(evt->cdevice.which);
	return 1;
}

static int win_oncontrollerremoved(event_t *evt)
{
	ctr_remv_device(evt->cdevice.which);
	return 1;
}

static int win_onwindowresize(event_t *evt)
{
	short i;

	if(evt){/* Prevent warning for not using parameter */}

	SDL_GetWindowSize(g_win.win, &g_win.win_w, &g_win.win_h);

	ren_resize(g_win.win_w, g_win.win_h);

	ui_adjust(g_win.root);

	for(i = 0; i < g_win.pipe_num; i++)
		ui_update(g_win.pipe[i]);

	return 1;
}

extern int win_proc_evt(event_t *evt)
{
	if(g_win.active != NULL) {
		struct ui_node *act;
		ui_node_fnc fnc;

		act = g_win.active;
		fnc = NULL;

		if(evt->type == SDL_KEYDOWN && evt->key.keysym.sym == 9) {
			if(act->next != NULL) {
					win_unfocus_node();
					win_focus_node(act->next);
			}
		}

		switch(evt->type) {
			case(SDL_KEYDOWN): fnc = act->events.keydown; break;
			case(SDL_KEYUP): fnc = act->events.keyup; break;
			case(SDL_TEXTINPUT): fnc = act->events.textinput; break;
		}

		if(fnc != NULL) {
			fnc(act, evt);
		}
	}

	switch(evt->type) {
		case(SDL_WINDOWEVENT):
			if(evt->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				return win_onwindowresize(evt);
			}
			break;

		case(SDL_MOUSEMOTION):
			return win_onmousemotion(evt);
			break;

		case(SDL_MOUSEBUTTONDOWN):
			return win_onmousebuttondown(evt);
			break;

		case(SDL_CONTROLLERDEVICEADDED):
			return win_oncontrolleradded(evt);
			break;

		case(SDL_CONTROLLERDEVICEREMOVED):
			return win_oncontrollerremoved(evt);
			break;

	}

	return -1;
}

static void win_collect_nodes(ui_node *n, void *d)
{
	short *idx = (short*)d;

	if(n->surf != NULL) {
		g_win.pipe[*idx] = n;
		*idx += 1;
	}
}

extern void win_build_pipe(void)
{
	g_win.pipe_num = 0;
	ui_down(g_win.root, &win_collect_nodes, &g_win.pipe_num, 0);
}

extern void win_dump_pipe(void)
{
	int i;
	ui_node *n;

	for(i = 0; i < g_win.pipe_num; i++) {
		n = g_win.pipe[i];
		printf("%s, vao %d, tex %d, Size(%d/%d)\n", n->id, n->vao, 
				n->tex, n->surf->w, n->surf->h);
	}
}

extern struct ui_node *win_check_hover(struct ui_node *n, int2_t pos)
{
	int i;
	rect_t *rend = &n->body;

	if(n->flags.active == 0)
		return NULL;

	if(pos[0] >= rend->x && pos[0] <= rend->x + rend->w &&
			pos[1] >= rend->y && pos[1] <= rend->y + rend->h) {

		if(n->flags.procevt == 1) {
			return n;
		}
	}

	for(i = 0; i < n->child_num; i++) {
		struct ui_node *ch = n->children[i];
		struct ui_node *ret = win_check_hover(ch, pos);
		if(ret != NULL)
			return ret; 
	}

	return NULL;
}

extern void win_focus_node(struct ui_node *n)
{
	ui_node *a = n;

	g_win.active = n;

	if(a != NULL && a->events.focus != NULL)
		a->events.focus(a, NULL);
}

extern void win_unfocus_node(void)
{
	ui_node *n = g_win.active;

	if(n != NULL && n->events.unfocus != NULL)
		n->events.unfocus(n, NULL);

	g_win.active = NULL;
}
