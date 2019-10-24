/* Using SDL and standard IO */
#include <SDL2/SDL.h>
#include <stdio.h>
/* Include custom headers */
#include "../XSDL/xsdl.h"
#include "vector.h"

/* Screen dimension */
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

/* === Global variables === */
char running = 0;					/* 1 if game is running, 0 if not */
SDL_Window *win;					/* Pointer to window-struct */
SDL_Renderer *ren;					/* Pointer to renderer-struct */
XSDL_Context *ctx;					/* The GUI-context */

SDL_Color clr = {0x18, 0x18, 0x18, 0xff};		/* Clear-color */

/* === Prototypes === */
void process_input();

void demofunc(XSDL_Node *node)
{
	SDL_Color newc = {
		rand() % 255,
		rand() % 255,
		rand() % 255,
		0xff
	};
	XSDL_ModStyle(node, STY_BCK_COL, &newc);
}

int main(int argc, char** args) 
{
	/* Initialize SDL */
	if(XSDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("[!] SDL could not initialize! (%s)\n", SDL_GetError());
		goto exit;
	}

	/* Create and initialize the window */
	win = SDL_CreateWindow("Vasall", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			SCREEN_WIDTH, SCREEN_HEIGHT, 
			SDL_WINDOW_SHOWN);
	if(win == NULL) {
		printf("[!] Window could not be created! (%s)\n", SDL_GetError());
		goto cleanup_sdl;
	}

	/* Set the window-icon */
	XSDL_SetWindowIcon(win);

	/* Create renderer */
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if(ren == NULL) {
		printf("[!] Renderer could not be created! (%s)\n", SDL_GetError());
		goto cleanup_window;
	}

	/* Create the GUI-context */
	ctx = XSDL_CreateContext(SCREEN_WIDTH, SCREEN_HEIGHT);
	if(ctx == NULL) {
		printf("[!] Context could not be created!\n");
		goto cleanup_renderer;
	}

	/* Attach a button to the context on the first level */
	XSDL_CreateButton(ctx->root, "button0", 20, 40, 120, 30, NULL);
	XSDL_CreateButton(ctx->root, "button1", 220, 40, 120, 30, NULL);

	SDL_Color c = {0xff, 0x00, 0x00, 0xff};
	XSDL_ModStyle(XSDL_Get(ctx->root, "button0"), STY_BCK_COL, &c);

	XSDL_BindEvent(XSDL_Get(ctx->root, "button1"), EVT_MOUSEDOWN, &demofunc);

	/* Build render-pipe */
	XSDL_BuildPipe(ctx->pipe, ctx->root);

	/* Mark game running */
	running = 1;

	/* Run the game */
	while(running) {
		/* Process user-input */
		process_input();

		/* Clear the screen */
		SDL_SetRenderDrawColor(ren, clr.r, clr.g, clr.b, 
				clr.a );
		SDL_RenderClear(ren);

		/* Render the current context */
		XSDL_RenderPipe(ren, ctx->pipe);

		/* Render all elements in the active scene */
		SDL_RenderPresent(ren);
	}

	/* Destroy context */
	XSDL_DeleteContext(ctx);

cleanup_renderer:
	/* Destory renderer */
	SDL_DestroyRenderer(ren);
cleanup_window:
	/* Destroy window */
	SDL_DestroyWindow(win);
cleanup_sdl:
	/* Quit SDL subsystems */
	SDL_Quit();
exit:
	return (0);
}

/**
 * Process a user-input. This function processes all 
 * possible inputs, like closing the window, pressing a
 * key or pressing a mouse-button. 
 */
void process_input() 
{
	SDL_Event event;

	/* Poll for events. SDL_PollEvent() returns 0 when there are no  */
	/* more events on the event queue, our while loop will exit when */
	/* that occurs.                                                  */
	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_QUIT) {
			running = 0;
			continue;
		}

		/* Process interactions with the UI */
		if(XSDL_ProcEvent(ctx, &event) < 0)
			continue;

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
		}
	}
}
