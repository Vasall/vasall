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
XSDL_Color CLR_COL = {0x18, 0x18, 0x18};		/* Background-color of window */

/* === Global variables === */
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

int main(int argc, char** args) 
{
	if(XSDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("[!] SDL could not initialize! (%s)\n", XSDL_GetError());
		goto exit;
	}

	if((window = init_window()) == NULL) {
		printf("[!] Window could not be created! (%s)\n", SDL_GetError());
		goto cleanup_sdl;
	}

	if((renderer = init_renderer(window)) == NULL) {
		printf("[!] Renderer could not be created! (%s)\n", SDL_GetError());
		goto cleanup_window;
	}

	if((context = XSDL_CreateContext(SCREEN_WIDTH, SCREEN_HEIGHT)) == NULL) {
		printf("[!] Context could not be created!\n");
		goto cleanup_renderer;
	}
	root = context->root;

	if(init_resources() < 0) {
		printf("[!] Couldn't load all resources!\n");
		goto cleanup_renderer;
	}

	init_gui(context);

	/* Display nodes in the console */
	display_nodes(context);

	/* Build render-pipe */
	XSDL_BuildPipe(context->pipe, root);

	/* Mark game as running */
	running = 1;	

	/* Run the game */
	while(running) {
		/* Process user-input */
		process_input();

		/* Clear the screen */
		SDL_SetRenderDrawColor(renderer, CLR_COL.r, CLR_COL.g, 
				CLR_COL.b, CLR_COL.a);
		SDL_RenderClear(renderer);

		XSDL_Update(context);

		/* Render the current UI-context */
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
			SDL_WINDOW_SHOWN);

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
	uint32_t flg = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

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
	readlink("/proc/self/exe", cwd, 256);	

	printf("Path: %s\n", cwd);

	char path[512];
	sprintf(path, "%s/%s", cwd, "mecha.ttf");	
	if(XSDL_LoadFont("/home/juke/code/c/vasall-client/res/bitter.ttf", 24) < 0)
		return (-1);
	if(XSDL_LoadFont("/home/juke/code/c/vasall-client/res/bitter.ttf", 16) < 0)
		return (-1);


	return(0);
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
	XSDL_CreateWrapper(rootnode, "menu", 
			0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	XSDL_CreateWrapper(XSDL_Get(rootnode, "menu"), "mns_form",
			200, 125, 400, 350);

	XSDL_Rect body = {250, 206, 300, 24};
	XSDL_Color col = {0xff, 0xff, 0xff};
	XSDL_CreateText(XSDL_Get(rootnode, "mns_form"), "label1", &body,
		"Email:", &col, 1, XSDL_TEXT_LEFT);
	XSDL_CreateInput(XSDL_Get(rootnode, "mns_form"), "mns_user", 
			250, 230, 300, 40, "");

	XSDL_Rect body1 = {250, 286, 300, 24};
	XSDL_Color col1 = {0xff, 0xff, 0xff};
	XSDL_CreateText(XSDL_Get(rootnode, "mns_form"), "label2", &body1,
		"Password:", &col1, 1, XSDL_TEXT_LEFT);
	XSDL_CreateInput(XSDL_Get(rootnode, "mns_form"), "mns_pswd", 
			250, 310, 300, 40, "");
	
	XSDL_CreateButton(XSDL_Get(rootnode, "mns_form"), "mns_login", 
			250, 380, 300, 40, "Login");

	uint8_t vis = 1;
	uint8_t bck = 1;
	XSDL_Color bck_col = {0xff, 0x00, 0x00};
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_form"), XSDL_STY_VIS, &vis);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_form"), XSDL_STY_BCK, &bck);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_form"), XSDL_STY_BCK_COL, &bck_col);
}

/*
 * Show a single node in the node-tree. This
 * function is a callback-function, therefore
 * the data-parameter has to be defined, despite
 * it not being used.
 *
 * @node: The node to display
 * @data: Data-pointer
*/
static void show_node(XSDL_Node *node, void *data)
{
	printf(" ");
	int i;
	for(i = 0; i < node->layer; i++) printf("  ");
	printf("%s:%d (0x%02x)", node->strid, node->id, node->tag);
	printf("\n");
}

/*
 * Display all nodes in a node tree.
 *
 * @ctx: The context containing the node-tree 
*/
void display_nodes(XSDL_Context *ctx)
{
	printf("\nNODE-TREE:\n");
	XSDL_GoDown(ctx->root, &show_node, NULL, 0);
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
		if(event.type == SDL_QUIT) {
			running = 0;
			return;
		}

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
