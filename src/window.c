#include "window.h"

#include <stdlib.h>


struct window_wrapper *window = NULL;


extern int win_init(void)
{
	int win_flgs = XSDL_WINDOW_RESIZABLE | XSDL_WINDOW_OPENGL;

	if(!(window = malloc(sizeof(struct window_wrapper))))
		return -1;

	memset(window, 0, sizeof(struct window_wrapper));

	if(XSDL_Init(XSDL_INIT_EVERYTHING) < 0)
		goto err_free_window;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_PROFILE_CORE);

	if(!(window->win = XSDL_CreateWindow("Vasall",
					XSDL_WINDOWPOS_UNDEFINED,
					XSDL_WINDOWPOS_UNDEFINED,
					800, 600, win_flgs)))
		goto err_close_xsdl;

	if(!(window->gl_ctx = XSDL_GL_CreateContext(window->win)))
		goto err_close_window;

	glClearColor(0.06, 0.06, 0.06, 1.0);
	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	if(!(window->ui_ctx = XSDL_CreateUIContext(window->win)))
		goto err_del_gl_ctx;

	window->ui_root = window->ui_ctx->root;
	return 0;

err_del_gl_ctx:
	XSDL_GL_DeleteContext(window->gl_ctx);

err_close_window:
	XSDL_DestroyWindow(window->win);

err_close_xsdl:
	XSDL_Quit();

err_free_window:
	free(window);
	return -1;
}

extern void win_close(void)
{
	if(!window)
		return;

	XSDL_ClearFontCache();
	XSDL_DeleteUIContext(window->ui_ctx);
	XSDL_GL_DeleteContext(window->gl_ctx);
	XSDL_DestroyWindow(window->win);
	XSDL_Quit();

	free(window);
	window = NULL;
}
