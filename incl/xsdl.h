#ifndef _XSDL_H_
#define _XSDL_H_
#include <SDL.h>

/*
 * A simple button-function.
*/
typedef struct XSDL_Button {
	SDL_Rect body;					// The underlying element-body
	void (*ptr)();					// Pointer to the callback-function
} XSDL_Button;


typedef struct XSDL_Input {
	SDL_Rect body;					// The underlying element-body
	char content[256];				// The current content in the Input	
} XSDL_Input;


typedef struct XSDL_Scene {
	short count;
} XSDL_Scene;


/* ==== DEFINE PROTOTYPES ==== */
void XSDL_RenderButton(SDL_Renderer *render, XSDL_Button *btn);

#endif
