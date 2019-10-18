#ifndef _XSDL_H_
#define _XSDL_H_

/*
 * A simple 2d-Vector struct used the
 * same as CORD, but with 32-bit numbers
 * instead of 16.
*/
typedef struct Vec2 {
	int x;
	int y;
} Vec2;

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

#endif
