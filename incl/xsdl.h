#ifndef _XSDL_H_
#define _XSDL_H_

#include <SDL.h>

#define PTR_SIZE 8
#define MAX_ELE_COUNT 15
#define ELE_BUF_LEN	(MAX_ELE_COUNT * PTR_SIZE)

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
	unsigned short count;			// The amount of elements in the scene
	char elements[ELE_BUF_LEN];		// A buffer containing all element-pointers
} XSDL_Scene;


/* ==== DEFINE PROTOTYPES ==== */
// Functions used to initialize the window
void XSDL_SetWindowIcon();

// Functions to manage a scene and create scene-elements
XSDL_Scene *XSDL_CreateScene();
void XSDL_DeleteScene(XSDL_Scene *scn);
int XSDL_CreateButton(XSDL_Scene *scn, int x, int y, int w, int h, void (*ptr)());

// Functions used to display scene-elements
void XSDL_RenderButton(SDL_Renderer *render, XSDL_Button *btn);

#endif
