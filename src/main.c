/* Using SDL and standard IO */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
/* Include custom headers */
#include "vector.h"

/* Window setting */
int win_w = 800;					/* Screen width */
int win_h = 600;					/* Screen height */
uint32_t win_flgs = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

/* === Global variables === */
uint8_t one = 1;
uint8_t zero = 0;

uint32_t running = 0;					/* 1 if game is running, 0 if not */
uint32_t fullscreen = 0;
SDL_Window *window;					/* Pointer to window-struct */
SDL_GLContext *gl_ctx;					/* Pointer to GL-context-struct */

/* === Prototypes === */
SDL_Window *init_window();
SDL_GLContext *init_context(SDL_Window *win);

int main(int argc, char** argv) 
{
	printf("\nStarting vasall-client...\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("[!] SDL could not initialize! (%s)\n", SDL_GetError());
		return(-1);
	}
	printf("\n");

	if((window = init_window()) == NULL) {
		printf("[!] Window could not be created! (%s)\n", SDL_GetError());
		return(-1);
	}

	if((gl_ctx = init_context(window)) == NULL) {
		printf("[!] GL-Context could not be created!\n");
		return(-1);
	}

	running = 1;

	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE:
						running = 0;
						break;
					case 'f':
						fullscreen = !fullscreen;
						if (fullscreen) {
							SDL_SetWindowFullscreen(window, win_flgs | SDL_WINDOW_FULLSCREEN_DESKTOP);
						}
						else {
							SDL_SetWindowFullscreen(window, win_flgs);
						}
						break;
					default:
						break;
				}
			}
			else if (event.type == SDL_QUIT) {
				running = 0;
			}
		}

		glViewport(0, 0, win_w, win_h);
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		SDL_GL_SwapWindow(window);
	}
exit:
	return (0);
}

/* ================= DEFINE FUNCTIONS ================ */

/*
 * Initialize the window and configure
 * basic settings like the title, minimal
 * window size and window-icon.
 *
 * Returns: Window-Pointer or NULL if an error occurred
 */
SDL_Window *init_window()
{
	/* Create and initialize the window */
	SDL_Window *win = SDL_CreateWindow("Vasall", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			win_w, win_h, 
			win_flgs);

	if(win == NULL) return (NULL);

	assert(win);

	return(win);
}

/*
 * This function will initialize the SDL-renderer
 * and set the render-flags. Note that it's very important
 * to enable VSync, or otherwise the either the Framerate
 * or performance will be off.
 *
 * @win: Pointer to underlying window
 *
 * Returns: Renderer-pointer or NULL if an error occurred
 */
SDL_GLContext *init_context(SDL_Window *win)
{
	SDL_GLContext ctx = SDL_GL_CreateContext(win);

	return(ctx);
}

#if DEBUG_TEST

/*
 * Load all necessary resources, like fonts, sprites
 * and more.
 *
 * Returns: 0 on success and -1 if an error occurred
 */
int init_resources()
{
	char cwd[256];
	char exe_dir[256];
	char path[512];
	int i = 0;

	readlink("/proc/self/exe", cwd, 256);
	for(i = strlen(cwd); i >= 0; i--)
		if(cwd[i] == '/')
			break;
	memcpy(exe_dir, cwd, i);

	printf("Directory: %s\n", exe_dir);

	sprintf(path, "%s/%s", exe_dir, "res/mecha.ttf");	
	if(XSDL_LoadFont(path, 24) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/aller.ttf");
	if(XSDL_LoadFont(path, 16) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/editundo.ttf");
	if(XSDL_LoadFont(path, 48) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/mns_bck.png");
	if(XSDL_LoadImage(renderer, path) < 0)
		goto loadfailed;

	return(0);

loadfailed:
	return(-1);
}

/*
 * Initialize the GUI and place all elements.
 *
 * @ctx: The context to create
 */
void init_gui(XSDL_UIContext *ctx)
{
	XSDL_Node *rootnode = ctx->root;

	/* Create the menu-sceen */
	XSDL_CreateWrapper(rootnode, "mns", 0, 0, -100, -100);
	XSDL_Node *mns_form = XSDL_CreateWrapper(XSDL_Get(rootnode, "mns"), 
			"mns_form", -1, -1, 400, 380);

	XSDL_CreateWrapper(mns_form, "mns_title",
			0, 0, 400, 80);
	XSDL_Rect body0 = {50, 14, 300, 52};
	XSDL_CreateText(XSDL_Get(rootnode, "mns_title"), "label0", &body0,
			"VASALL", &XSDL_WHITE, 2, 0);

	XSDL_Rect body1 = {40, 106, 320, 24};
	XSDL_CreateText(mns_form, "mns_user_label", &body1,"Email:", 
			&XSDL_WHITE, 1, XSDL_TEXT_LEFT);
	XSDL_CreateInput(mns_form, "mns_user", 40, 130, 320, 40, "");

	XSDL_Rect body2 = {40, 186, 320, 24};
	XSDL_CreateText(mns_form, "mns_user_label", &body2, "Password:", 
			&XSDL_WHITE, 1, XSDL_TEXT_LEFT);
	XSDL_CreateInput(mns_form, "mns_pswd", 40, 210, 320, 40, "");

	XSDL_CreateButton(XSDL_Get(rootnode, "mns_form"), "mns_login", 
			40, 280, 320, 40, "Login");

	XSDL_Color mns_form_bck_col = {0x23, 0x23, 0x23, 0xff};
	short mns_form_corners[] = {5, 5, 5, 5};
	XSDL_ModStyle(mns_form, XSDL_STY_VIS, &one);
	XSDL_ModStyle(mns_form, XSDL_STY_BCK, &one);
	XSDL_ModStyle(mns_form, XSDL_STY_BCK_COL, &mns_form_bck_col);
	XSDL_ModStyle(mns_form, XSDL_STY_COR_RAD, &mns_form_corners);

	XSDL_Color mns_title_bck_col = {0xd3, 0x34, 0x5a, 0xff};
	short mns_title_cor[] = {5, 5, 0, 0};
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_BCK_COL, &mns_title_bck_col);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_COR_RAD, &mns_title_cor);

	XSDL_BindEvent(XSDL_Get(rootnode, "mns_login"), XSDL_EVT_MOUSEDOWN, &try_login);

	/* Create the game-sceen */
	XSDL_CreateWrapper(rootnode, "gms", 0, 0, -100, -100);
	XSDL_Color gms_bck_col = {0x47, 0x2d, 0x5c, 0xff};
	XSDL_ModStyle(XSDL_Get(rootnode, "gms"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms"), XSDL_STY_BCK_COL, &gms_bck_col);

	XSDL_CreateWrapper(XSDL_Get(rootnode, "gms"), "gms_stats", -1, 5, 780, 35);
	XSDL_Color gms_stats_bck_col = {0x23, 0x25, 0x30, 0xff};
	short gms_stats_cor[] = {6, 6, 6, 6};
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_BCK_COL, &gms_stats_bck_col);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_COR_RAD, &gms_stats_cor);

	XSDL_ModFlag(XSDL_Get(rootnode, "gms"), XSDL_FLG_ACT, &zero);

	XSDL_ShowNodes(rootnode);	
}

#endif
