#include <stdio.h>
#include <SDL.h>
#include "xsdl.h"

/*
 * Render a button on the screen.
 *
 * @render: The renderer used to display the button
 * @btn: The button-struct to display
*/
void XSDL_RenderButton(SDL_Renderer *render, XSDL_Button *btn) {
	SDL_SetRenderDrawColor(render, 0xff, 0x00, 0x00, 0xff);
	SDL_RenderFillRect(render, &btn->body);
}



