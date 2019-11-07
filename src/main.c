/* Using SDL and standard IO */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <unistd.h>
/* Include custom headers */
#include "../XSDL/xsdl.h"
#include "vector.h"

/* Window setting */
int SCREEN_WIDTH = 800;					/* Screen width */
int SCREEN_HEIGHT = 600;				/* Screen height */
XSDL_Color clr = { 0x18, 0x18, 0x18};			/* Background-color of window */

/* === Global variables === */
uint8_t one = 1;
uint8_t zero = 0;

char running = 0;					/* 1 if game is running, 0 if not */
XSDL_Window *window;					/* Pointer to window-struct */
XSDL_Renderer *renderer;				/* Pointer to renderer-struct */
XSDL_Context *context;					/* The GUI-context */
XSDL_Node *root;					/* Pointer to the root node */

/* === Prototypes === */
XSDL_Window *init_window();
XSDL_Renderer *init_renderer(XSDL_Window *win);
int init_resources();
void init_gui(XSDL_Context *ctx);
void display_nodes(XSDL_Context *ctx);
void process_input();
void try_login();


int main(int argc, char** argv) 
{
	printf("\nStarting vasall-client...\n");

	if(XSDL_Init(XSDL_INIT_EVERYTHING) < 0) {
		printf("[!] SDL could not initialize! (%s)\n", XSDL_GetError());
		goto exit;
	}
	XSDL_ShowVersions();
	printf("\n");

	if((window = init_window()) == NULL) {
		printf("[!] Window could not be created! (%s)\n", SDL_GetError());
		goto cleanup_sdl;
	}

	if((renderer = init_renderer(window)) == NULL) {
		printf("[!] Renderer could not be created! (%s)\n", SDL_GetError());
		goto cleanup_window;
	}

	if((context = XSDL_CreateContext(window)) == NULL) {
		printf("[!] Context could not be created!\n");
		goto cleanup_renderer;
	}
	root = context->root;

	if(init_resources(argv[0]) < 0) {
		printf("[!] Couldn't load all resources!\n");
		goto cleanup_renderer;
	}

	/* Initialize the user-interface */
	init_gui(context);

	/* Mark game as running */
	running = 1;

	/* Run the game */
	while(running) {
		/* -------- UPDATE -------- */
		/* Process user-input */
		process_input();

		/* -------- RENDER -------- */
		/* Clear the screen */
		SDL_SetRenderDrawColor(renderer, clr.r, clr.g,
				clr.b, clr.a);
		SDL_RenderClear(renderer);

		/* Update UI-nodes */
		XSDL_Update(context);

		/* Render visible UI-elements */
		XSDL_RenderPipe(renderer, context->pipe);

		/* Render all elements in the active scene */
		SDL_RenderPresent(renderer);
	}

	/* Destroy context */
	XSDL_DeleteContext(context);

cleanup_renderer:
	/* Destory renderer */
	SDL_DestroyRenderer(renderer);
cleanup_window:
	/* Destroy window */
	SDL_DestroyWindow(window);
cleanup_sdl:
	/* Quit XSDL subsystems */
	XSDL_Quit();
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
XSDL_Window *init_window()
{
	/* Create and initialize the window */
	XSDL_Window *win = SDL_CreateWindow("Vasall", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			SCREEN_WIDTH, SCREEN_HEIGHT, 
			SDL_WINDOW_RESIZABLE);

	if(win == NULL) return (NULL);

	/* Set the window-icon */
	XSDL_SetWindowIcon(win);

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
XSDL_Renderer *init_renderer(XSDL_Window *win)
{
	uint32_t flg = XSDL_RENDERER_ACCELERATED | 
		XSDL_RENDERER_PRESENTVSYNC;

	/* Create and initialize the renderer*/
	XSDL_Renderer *ren = SDL_CreateRenderer(win, -1, flg);

	return(ren);
}

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
void init_gui(XSDL_Context *ctx)
{
	XSDL_Node *rootnode = ctx->root;

	/* Create the menu-sceen */
	XSDL_CreateWrapper(rootnode, "mns", 0, 0, -100, -100);
	XSDL_CreateWrapper(XSDL_Get(rootnode, "mns"), "mns_form",
			-1, -1, 400, 380);

	XSDL_CreateWrapper(XSDL_Get(rootnode, "mns_form"), "mns_title",
			0, 0, 400, 80);
	XSDL_Rect body0 = {50, 14, 300, 52};
	XSDL_Color col0 = {0xff, 0xff, 0xff, 0xff};
	XSDL_CreateText(XSDL_Get(rootnode, "mns_title"), "label0", &body0,
		"VASALL", &col0, 2, 0);

	XSDL_Rect body1 = {40, 106, 320, 24};
	XSDL_Color col1 = {0xff, 0xff, 0xff, 0xff};
	XSDL_CreateText(XSDL_Get(rootnode, "mns_form"), "label1", &body1,
		"Email:", &col1, 1, XSDL_TEXT_LEFT);
	XSDL_CreateInput(XSDL_Get(rootnode, "mns_form"), "mns_user", 
			40, 130, 320, 40, "");

	XSDL_Rect body2 = {40, 186, 320, 24};
	XSDL_Color col2 = {0xff, 0xff, 0xff, 0xff};
	XSDL_CreateText(XSDL_Get(rootnode, "mns_form"), "label2", &body2,
		"Password:", &col2, 1, XSDL_TEXT_LEFT);
	XSDL_CreateInput(XSDL_Get(rootnode, "mns_form"), "mns_pswd", 
			40, 210, 320, 40, "");
	
	XSDL_CreateButton(XSDL_Get(rootnode, "mns_form"), "mns_login", 
			40, 280, 320, 40, "Login");

	XSDL_Color bck_col = {0x23, 0x23, 0x23, 0xff};
	short form_corners[] = {5, 5, 5, 5};
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_form"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_form"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_form"), XSDL_STY_BCK_COL, &bck_col);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_form"), XSDL_STY_COR_RAD, &form_corners);

	XSDL_Color title_col = {0xd3, 0x34, 0x5a, 0xff};
	short title_cor[] = {5, 5, 0, 0};
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_BCK_COL, &title_col);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_COR_RAD, &title_cor);

	XSDL_BindEvent(XSDL_Get(rootnode, "mns_login"), XSDL_EVT_MOUSEDOWN, &try_login);

	/* Create the game-sceen */
	XSDL_CreateWrapper(rootnode, "gms", 0, 0, -100, -100);
	XSDL_Color gms_bck_col = {0x47, 0x2d, 0x5c, 0xff};
	XSDL_ModStyle(XSDL_Get(rootnode, "gms"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms"), XSDL_STY_BCK_COL, &gms_bck_col);

	XSDL_CreateWrapper(XSDL_Get(rootnode, "gms"), "gms_stats", -1, 5, 780, 30);
	XSDL_Color test_col = {0x23, 0x25, 0x30, 0xff};
	short stats_cor[] = {6, 6, 6, 6};
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_BCK_COL, &test_col);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_COR_RAD, &stats_cor);

	XSDL_ModFlag(XSDL_Get(rootnode, "gms"), XSDL_FLG_ACT, &zero);

	XSDL_ShowNodes(rootnode);	
}

/**
 * Process a user-input. This function processes all 
 * possible inputs, like closing the window, pressing a
 * key or pressing a mouse-button. 
 */
void process_input() 
{
	XSDL_Event event;

	/* Poll for events. SDL_PollEvent() returns 0 when there are no  */
	/* more events on the event queue, our while loop will exit when */
	/* that occurs.                                                  */
	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_QUIT)
			running = 0;

		/* Process interactions with the UI */
		if(XSDL_ProcEvent(context, &event) > -1)
			continue;

		/* If user didn't interact with UI */
		switch(event.type) {
			case(SDL_MOUSEMOTION):
				break;

			case(SDL_MOUSEBUTTONDOWN):
				switch(event.button.button) {
					/* Left mouse-button */
					case(1):
						break;

					/* Right mouse-button */
					case(3):
						break;
				}
				break;

			case(SDL_KEYDOWN):
				switch(event.key.keysym.sym) {
					case(SDLK_ESCAPE):
						running = 0;
						break;
				}
				break;
		}
	}
}

void try_login()
{
	printf("Logged in.\n");

	XSDL_ModFlag(XSDL_Get(root, "mns"), XSDL_FLG_ACT, &zero);
	XSDL_ModFlag(XSDL_Get(root, "gms"), XSDL_FLG_ACT, &one);

	XSDL_ShowNodes(root);
	XSDL_ShowPipe(context);
}
