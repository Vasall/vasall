#include <stdio.h>
#include <SDL.h>
#include "xsdl.h"

void XSDL_RenderButton(SDL_Renderer *render, XSDL_Button *btn) {
	SDL_SetRenderDrawColor(render, 0xff, 0x00, 0x00, 0xff);
	SDL_RenderFillRect(render, &btn->body);
}



