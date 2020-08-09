#include "window.h"

#include <stdlib.h>

/* Redefine global window-wrapper */
struct window_wrapper window;

const char *vtxShd = "#version 330 core\n \
		      in vec3 pos; in vec2 tex; out vec2 uv; \
		      void main(){gl_Position = vec4(pos, 1.0); uv = tex;}";

const char *frgShd = "#version 330 core\n \
		      uniform sampler2D tex; in vec2 uv; out vec4 col; \
		      void main(){col = texture(tex, uv);}";



static int win_conf_opengl(void)
{
	/* glClearColor(0.196, 0.235, 0.282, 1.0); */
	glClearColor(0.094, 0.094, 0.094, 1.0);
	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	return 0;
}

static int win_setup_shader(void)
{
	uint32_t vshd, fshd;
	int success;

	if((window.shader = glCreateProgram()) == 0)
		return -1;

	vshd = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshd, 1, (const GLchar **)&vtxShd, NULL);
	glCompileShader(vshd);

	glGetShaderiv(vshd, GL_COMPILE_STATUS, &success);
	if(!success) {
		glDeleteShader(vshd);
		goto err_del_prog;
	}

	glAttachShader(window.shader, vshd);

	fshd = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshd, 1, (const GLchar **)&frgShd, NULL);
	glCompileShader(fshd);

	glGetShaderiv(fshd, GL_COMPILE_STATUS, &success);
	if(!success) {
		glDeleteShader(vshd);
		glDeleteShader(fshd);
		goto err_del_prog;
	}

	glAttachShader(window.shader, fshd);

	/* Bind the vertex-attributes */
	glBindAttribLocation(window.shader, 0, "pos");
	glBindAttribLocation(window.shader, 1, "tex");

	glLinkProgram(window.shader);

	glDetachShader(window.shader, vshd);
	glDeleteShader(vshd);

	glDetachShader(window.shader, fshd);
	glDeleteShader(fshd);
	return 0;

err_del_prog:
	glDeleteProgram(window.shader);
	return -1;
}

static int win_load_cursors(void)
{
	window.cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	window.cursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	window.cursors[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	return 0;
}

extern int win_init(void)
{
	int win_flgs = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	struct ui_node *root = NULL;

	if(!(window.win = SDL_CreateWindow("Vasall",
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					WIN_W, WIN_H, win_flgs)))
		return -1;

	if(!(window.gl_ctx = SDL_GL_CreateContext(window.win)))
		goto err_close_window;

	if(win_conf_opengl() < 0)
		goto err_delete_context;

	if(win_setup_shader() < 0)
		goto err_delete_context;

	window.hover = NULL;
	window.active = NULL;

	window.win_w = WIN_W;
	window.win_h = WIN_H;

	if(!(window.root = ui_add(UI_WRAPPER, NULL, NULL, "root")))
		goto err_delete_shader;
		
	win_build_pipe();
	
	if(win_load_cursors() < 0)
		goto err_delete_root;

	return 0;

err_delete_root:
	ui_remv(root);

err_delete_shader:
	glDeleteProgram(window.shader);

err_delete_context:
	SDL_GL_DeleteContext(window.gl_ctx);	

err_close_window:
	SDL_DestroyWindow(window.win);
	window.win = NULL;
	return -1;
}

extern void win_close(void)
{
	ui_remv(window.root);
	glDeleteProgram(window.shader);
	SDL_GL_DeleteContext(window.gl_ctx);
	SDL_DestroyWindow(window.win);
}

extern void win_update(void)
{
	if(window.active) {
		if(window.active->events.onactive){
			window.active->events.onactive(window.active, NULL);
		}
	}
}

extern void win_render(void)
{
	short i;

	for(i = 0; i < window.pipe_num; i++)
		ui_render(window.pipe[i]);
}

static int win_onmousemotion(event_t *evt)
{
	int2_t pos;
	ui_node_fnc fnc;

	if(evt){/*Prevent warning from unused variable <evt>*/}

	SDL_GetMouseState(&pos[0], &pos[1]);
	window.hover = win_check_hover(window.root, pos);

	/* Hovering over element */
	if(window.hover != NULL) {
		struct ui_node *hovered = window.hover;

		/* Use flag-cursor */
		SDL_SetCursor(window.cursors[(int)hovered->flags.cur]);

		if((fnc = hovered->events.hover) != NULL)
			fnc(window.hover, NULL);

		return 1;
	}

	SDL_SetCursor(window.cursors[0]);
	return -1;
}

static int win_onmousebuttondown(event_t *evt)
{
	struct ui_node *hovered;

	if(window.hover == NULL) {
		if(window.active != NULL) {
			win_unfocus_node();
		}
		return  -1;
	}

	hovered = window.hover;

	switch(evt->button.button) {
		case(1):
			if(hovered->flags.enfoc == 1) {
				win_focus_node(hovered);
			}

			if(hovered->events.mousedown != NULL) {
				hovered->events.mousedown(hovered, evt);
			}

			return 1;

		case(3):
			break;
	}

	return -1;
}

static int win_oncontrolleradded(event_t *evt)
{
	inp_add_device(evt->cdevice.which);
	return 1;
}

static int win_oncontrollerremoved(event_t *evt)
{
	inp_remv_device(evt->cdevice.which);
	return 1;
}

static int win_onwindowresize(event_t *evt)
{
	short i;

	if(evt){/* Prevent warning for not using parameter */}

	SDL_GetWindowSize(window.win, &window.win_w, &window.win_h);

	glViewport(0, 0, window.win_w, window.win_h);

	ui_adjust(window.root);

	for(i = 0; i < window.pipe_num; i++)
		ui_update(window.pipe[i]);

	return 1;
}

extern int win_proc_evt(event_t *evt)
{
	if(window.active != NULL) {
		struct ui_node *act;
		ui_node_fnc fnc;

		act = window.active;
		fnc = NULL;

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
		window.pipe[*idx] = n;
		*idx += 1;
	}
}

extern void win_build_pipe(void)
{
	window.pipe_num = 0;
	ui_down(window.root, &win_collect_nodes, &window.pipe_num, 0);
}

extern void win_dump_pipe(void)
{
	int i;
	ui_node *n;

	for(i = 0; i < window.pipe_num; i++) {
		n = window.pipe[i];
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
	window.active = n;
}

extern void win_unfocus_node(void)
{
	window.active = NULL;
}
