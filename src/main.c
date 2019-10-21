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
char game_running = 0;					/* 1 if game is running, 0 if not */
SDL_Window *win = NULL;					/* Pointer to the window-struct */
SDL_Renderer *ren = NULL;				/* Pointer to the renderer-struct */
XSDL_Node *context;						/* Pointer to the super-node */
XSDL_Pipe *pipe;						/* Pointer to the render-pipe */
SDL_Color clr = {0x18, 0x18, 0x18, 0xff};

int hovered = -1;						/* Pointer to the hovered element */

/* === Prototypes === */
void process_input();
void demofunc() 
{
	clr.r = floor(rand() % 255);
	clr.g = floor(rand() % 255);
	clr.b = floor(rand() % 255);
}

int main(int argc, char** args) 
{
    /* Initialize SDL */
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[!] SDL could not initialize! (%s)\n", SDL_GetError());
		goto exit;
    }

	/* Create and initialize the window */
    if((win = SDL_CreateWindow("Vasall", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			SCREEN_WIDTH, SCREEN_HEIGHT, 
			SDL_WINDOW_SHOWN)) == NULL) {
		printf("[!] Window could not be created! (%s)\n", SDL_GetError());
		goto cleanup_sdl;
	}

	/* Set the window-icon */
	XSDL_SetWindowIcon(win);

	/* Create renderer */
	if((ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
		printf("[!] Renderer could not be created! (%s)\n", SDL_GetError());
		goto cleanup_window;
	}

	/* Initialize the first context */
	if((context = XSDL_CreateContext(0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)) == NULL) {
		printf("[!] Could not create context.\n");
		goto cleanup_renderer;
	}

	/* Initialize the render-pipe */
	if((pipe = XSDL_CreatePipe()) == NULL) {
		printf("[!] Could not create pipe.\n");
		goto cleanup_pipe;
	}

	/* Attach a button to the context on the first level */
	XSDL_CreateButton(context, 20, 40, 120, 30, &demofunc);
	XSDL_CreateButton(context, 220, 40, 120, 30, &demofunc);

	XSDL_BuildPipe(pipe, context);

	/* Mark game running */
	game_running = 1;

	/* Run the game */
	while(game_running) {
		process_input();

		SDL_SetRenderDrawColor(ren, clr.r, clr.g, clr.b, clr.a );

		SDL_RenderClear(ren);

		/* Render the current context */
		XSDL_RenderPipe(ren, pipe);
		
		/* Render all elements in the active scene */
		SDL_RenderPresent(ren);
	}

cleanup_pipe:
	/* Destroy context */
	XSDL_DeleteContext(context);
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
	int x, y;

	/* Poll for events. SDL_PollEvent() returns 0 when there are no  */
	/* more events on the event queue, our while loop will exit when */
  	/* that occurs.                                                  */
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case(SDL_QUIT):
				game_running = 0;
				break;
	
			case(SDL_MOUSEMOTION):
				SDL_GetMouseState(&x, &y);
				Vec2 pos = {x, y};


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
