// Using SDL and standard IO
#include <SDL.h>
#include <stdio.h>
// Include custom headers
#include "incl/xsdl.h"
#include "incl/xmath.h"

// Screen dimension
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;

// === Global variables ===
char game_running = 0;					// 1 if game is running, 0 if not
SDL_Window *win = NULL;					// Pointer to the window-struct
SDL_Renderer *ren = NULL;				// Pointer to the renderer-struct
XSDL_Button button;

// === Prototypes ===
void processInput();
void run();
void demoprint() { printf("TEST"); }


int main(int argc, char** args) {
    // The surface contained by the window
    SDL_Surface* screenSurface = NULL;

	// Create a button
	//SDL_Rect button_body = {10, 10, 100, 30};
	//button = {button_body, &demoprint};

    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[!] SDL could not initialize! (%s)\n", SDL_GetError());
		goto exit;
    }

	// Create window
    win = SDL_CreateWindow("TinyKindom", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			SCREEN_WIDTH, SCREEN_HEIGHT, 
			SDL_WINDOW_RESIZABLE);
    if(win == NULL) {
		printf("[!] Window could not be created! (%s)\n", SDL_GetError());
		goto closewindow;
	}

	// Create renderer
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if(ren == NULL) {
		printf("[!] Renderer could not be created! (%s)\n", SDL_GetError());
	}

	SDL_Color clr = {0x18, 0x18, 0x18};

	// Mark game running
	game_running = 1;

	// Run the game
	while(game_running) {

		processInput();

		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255 );

		SDL_RenderClear(ren);

		// Render a button
		// XSDL_RenderButton(ren, &button);
		// Render all elements in the active scene
		SDL_RenderPresent(ren);
	}

closewindow:
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
 * 
*/
void processInput() {
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
						//if(inRect(&button, &pos)) {
						//	printf("x: %d, y: %d\n", x, y);
						//}
						break;

					// Right mouse-button
					case(3):
						break;
				}
				break;
		}
	}
}
