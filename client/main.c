// Using SDL and standard IO
#include <SDL.h>
#include <stdio.h>
// Include custom headers
#include "xsdl.h"
#include "../shared/vector.h"

// Screen dimension
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

/* === Global variables === */
char game_running = 0;					// 1 if game is running, 0 if not
SDL_Window *win = NULL;					// Pointer to the window-struct
SDL_Renderer *ren = NULL;				// Pointer to the renderer-struct
XSDL_Button button;
XSDL_Scene *scene;
SDL_Color clr = {0x18, 0x18, 0x18, 0xff};

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
    // The surface contained by the window
    SDL_Surface* screenSurface = NULL;

	// Create a button
	SDL_Rect button_body = {10, 10, 100, 30};
	button.body.x = button_body.x;
	button.body.y = button_body.y;
	button.body.w = button_body.w;
	button.body.h = button_body.h;
	button.ptr = &demofunc;

    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[!] SDL could not initialize! (%s)\n", SDL_GetError());
		goto exit;
    }

	// Create and initialize the window
    if((win = SDL_CreateWindow("TinyKindom", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			SCREEN_WIDTH, SCREEN_HEIGHT, 
			SDL_WINDOW_SHOWN)) == NULL) {
		printf("[!] Window could not be created! (%s)\n", SDL_GetError());
		goto closewindow;
	}

	// Set the window-icon
	XSDL_SetWindowIcon(win);

	// Create renderer
	if((ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
		printf("[!] Renderer could not be created! (%s)\n", SDL_GetError());
		goto closewindow;
	}

	// Initialize the first scene
	if((scene = XSDL_CreateScene()) == NULL) {
		printf("[!] Could not create scene.\n");
		goto closewindow;
	}

	// XSDL_Add();
	XSDL_AddElement(scene, 1, &button);

	printf("Scene-Count: %d\n", scene->count);

	// Mark game running
	game_running = 1;

	// Run the game
	while(game_running) {
		process_input();

		SDL_SetRenderDrawColor(ren, clr.r, clr.g, clr.b, clr.a );

		SDL_RenderClear(ren);

		// Render a button
		XSDL_RenderButton(ren, &button);
		// Render all elements in the active scene
		SDL_RenderPresent(ren);
	}

closewindow:
	XSDL_DeleteScene(scene);
	// Destory renderer
	SDL_DestroyRenderer(ren);
	// Destroy window
    SDL_DestroyWindow(win);

exit:
    // Quit SDL subsystems
    SDL_Quit();
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

			case(SDL_MOUSEBUTTONDOWN):
				switch(event.button.button) {
					// Left mouse-button
					case(1):
						SDL_GetMouseState(&x, &y);
						Vec2 pos = {x, y};

						//if(in_rect(&button, &pos)) {
						//printf("x: %d, y: %d\n", x, y);
						//}
						
						if(in_rect(&button.body, &pos)) {
							button.ptr();
						}
						break;

					// Right mouse-button
					case(3):
						break;
				}
				break;
		}
	}
}
