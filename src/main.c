/* Using SDL and standard IO */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <unistd.h>
/* Include custom headers */
#include "../ENUD/enud.h"
#include "vector.h"

/* Window setting */
int SCREEN_WIDTH = 800;					/* Screen width */
int SCREEN_HEIGHT = 600;				/* Screen height */
ENUD_Color clr = { 0x18, 0x18, 0x18};			/* Background-color of window */

/* === Global variables === */
uint8_t one = 1;
uint8_t zero = 0;

char running = 0;					/* 1 if game is running, 0 if not */
ENUD_Window *window;					/* Pointer to window-struct */
ENUD_Renderer *renderer;				/* Pointer to renderer-struct */
ENUD_UIContext *context;				/* The GUI-context */
ENUD_Node *root;					/* Pointer to the root node */

/* === Prototypes === */
ENUD_Window *init_window();
ENUD_Renderer *init_renderer(ENUD_Window *win);
int init_resources();
void init_gui(ENUD_UIContext *ctx);
void display_nodes(ENUD_UIContext *ctx);
void process_input();
void try_login();


int main(int argc, char** argv) 
{
	printf("\nStarting vasall-client...\n");

	if(ENUD_Init(ENUD_INIT_EVERYTHING) < 0) {
		printf("[!] SDL could not initialize! (%s)\n", ENUD_GetError());
		goto exit;
	}
	ENUD_ShowVersions();
	printf("\n");

	if((window = init_window()) == NULL) {
		printf("[!] Window could not be created! (%s)\n", SDL_GetError());
		goto cleanup_sdl;
	}

	if((renderer = init_renderer(window)) == NULL) {
		printf("[!] Renderer could not be created! (%s)\n", SDL_GetError());
		goto cleanup_window;
	}

	if((context = ENUD_CreateUIContext(window)) == NULL) {
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
		ENUD_Update(context);

		/* Render visible UI-elements */
		ENUD_RenderPipe(renderer, context->pipe);

		/* Render all elements in the active scene */
		SDL_RenderPresent(renderer);
	}

	/* Destroy context */
	ENUD_DeleteUIContext(context);

cleanup_renderer:
	/* Destory renderer */
	SDL_DestroyRenderer(renderer);
cleanup_window:
	/* Destroy window */
	SDL_DestroyWindow(window);
cleanup_sdl:
	/* Quit ENUD subsystems */
	ENUD_Quit();
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
ENUD_Window *init_window()
{
	/* Create and initialize the window */
	ENUD_Window *win = SDL_CreateWindow("Vasall", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			SCREEN_WIDTH, SCREEN_HEIGHT, 
			SDL_WINDOW_RESIZABLE);

	if(win == NULL) return (NULL);

	/* Set the window-icon */
	ENUD_SetWindowIcon(win);

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
ENUD_Renderer *init_renderer(ENUD_Window *win)
{
	uint32_t flg = ENUD_RENDERER_ACCELERATED | 
		ENUD_RENDERER_PRESENTVSYNC;

	/* Create and initialize the renderer*/
	ENUD_Renderer *ren = SDL_CreateRenderer(win, -1, flg);

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
	if(ENUD_LoadFont(path, 24) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/aller.ttf");
	if(ENUD_LoadFont(path, 16) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/editundo.ttf");
	if(ENUD_LoadFont(path, 48) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/mns_bck.png");
	if(ENUD_LoadImage(renderer, path) < 0)
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
void init_gui(ENUD_UIContext *ctx)
{
	ENUD_Node *rootnode = ctx->root;

	/* Create the menu-sceen */
	ENUD_CreateWrapper(rootnode, "mns", 0, 0, -100, -100);
	ENUD_CreateWrapper(ENUD_Get(rootnode, "mns"), "mns_form",
			-1, -1, 400, 380);

	ENUD_CreateWrapper(ENUD_Get(rootnode, "mns_form"), "mns_title",
			0, 0, 400, 80);
	ENUD_Rect body0 = {50, 14, 300, 52};
	ENUD_CreateText(ENUD_Get(rootnode, "mns_title"), "label0", &body0,
		"VASALL", &ENUD_WHITE, 2, 0);

	ENUD_Rect body1 = {40, 106, 320, 24};
	ENUD_CreateText(ENUD_Get(rootnode, "mns_form"), "label1", &body1,"Email:", &ENUD_WHITE, 1, ENUD_TEXT_LEFT);
	ENUD_CreateInput(ENUD_Get(rootnode, "mns_form"), "mns_user", 40, 130, 320, 40, "");

	ENUD_Rect body2 = {40, 186, 320, 24};
	ENUD_CreateText(ENUD_Get(rootnode, "mns_form"), "label2", &body2, "Password:", &ENUD_WHITE, 1, ENUD_TEXT_LEFT);
	ENUD_CreateInput(ENUD_Get(rootnode, "mns_form"), "mns_pswd", 40, 210, 320, 40, "");
	
	ENUD_CreateButton(ENUD_Get(rootnode, "mns_form"), "mns_login", 40, 280, 320, 40, "Login");

	ENUD_Color mns_form_bck_col = {0x23, 0x23, 0x23, 0xff};
	short mns_form_corners[] = {5, 5, 5, 5};
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_form"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_form"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_form"), ENUD_STY_BCK_COL, &mns_form_bck_col);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_form"), ENUD_STY_COR_RAD, &mns_form_corners);

	ENUD_Color mns_title_bck_col = {0xd3, 0x34, 0x5a, 0xff};
	short mns_title_cor[] = {5, 5, 0, 0};
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_title"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_title"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_title"), ENUD_STY_BCK_COL, &mns_title_bck_col);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_title"), ENUD_STY_COR_RAD, &mns_title_cor);

	ENUD_BindEvent(ENUD_Get(rootnode, "mns_login"), ENUD_EVT_MOUSEDOWN, &try_login);

	/* Create the game-sceen */
	ENUD_CreateWrapper(rootnode, "gms", 0, 0, -100, -100);
	ENUD_Color gms_bck_col = {0x47, 0x2d, 0x5c, 0xff};
	ENUD_ModStyle(ENUD_Get(rootnode, "gms"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "gms"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "gms"), ENUD_STY_BCK_COL, &gms_bck_col);

	ENUD_CreateWrapper(ENUD_Get(rootnode, "gms"), "gms_stats", -1, 5, 780, 35);
	ENUD_Color gms_stats_bck_col = {0x23, 0x25, 0x30, 0xff};
	short gms_stats_cor[] = {6, 6, 6, 6};
	ENUD_ModStyle(ENUD_Get(rootnode, "gms_stats"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "gms_stats"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "gms_stats"), ENUD_STY_BCK_COL, &gms_stats_bck_col);
	ENUD_ModStyle(ENUD_Get(rootnode, "gms_stats"), ENUD_STY_COR_RAD, &gms_stats_cor);

	ENUD_ModFlag(ENUD_Get(rootnode, "gms"), ENUD_FLG_ACT, &zero);

	ENUD_ShowNodes(rootnode);	
}

/**
 * Process a user-input. This function processes all 
 * possible inputs, like closing the window, pressing a
 * key or pressing a mouse-button. 
 */
void process_input() 
{
	ENUD_Event event;

	/* Poll for events. SDL_PollEvent() returns 0 when there are no  */
	/* more events on the event queue, our while loop will exit when */
	/* that occurs.                                                  */
	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_QUIT)
			running = 0;

		/* Process interactions with the UI */
		if(ENUD_ProcEvent(context, &event) > -1)
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

	ENUD_ModFlag(ENUD_Get(root, "mns"), ENUD_FLG_ACT, &zero);
	ENUD_ModFlag(ENUD_Get(root, "gms"), ENUD_FLG_ACT, &one);

	ENUD_ShowNodes(root);
	ENUD_ShowPipe(context);
}
