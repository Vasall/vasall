#ifndef _WINDOW_H
#define _WINDOW_H

#include "sdl.h"
#include "node.h"
#include "vec.h"
#include "input.h"

#define PIPE_LEN 32

#define WIN_W 800
#define WIN_H 600

enum cursors {
	CURSOR_ARROW = 0,
	CURSOR_IBEAM = 1,
	CURSOR_HAND  = 2
};

struct window_wrapper {
	SDL_Window *win;
	SDL_GLContext gl_ctx;

	struct ui_node *root;
	
	short pipe_num;
	struct ui_node *pipe[PIPE_LEN];	

	struct ui_node *hover;
	struct ui_node *active;

	int win_w;
	int win_h;

	uint32_t shader;

	SDL_Cursor *cursors[8];
};


extern struct window_wrapper window;


extern int win_init(void);
extern void win_close(void);

extern void win_update(void);
extern void win_render(void);

extern int win_proc_evt(event_t *evt); 

extern void win_build_pipe(void);
extern void win_dump_pipe(void);

extern void win_focus_node(struct ui_node *n);
extern void win_unfocus_node(void);

extern struct ui_node *win_check_hover(struct ui_node *n, int2_t pos);

#endif
