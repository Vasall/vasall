#ifndef VASALL_WRAPPER_H
#define VASALL_WRAPPER_H

#include "../enud/enud.h"
#include "camera.h"
#include "world.h"

typedef struct gloWrapper {
	/*
	 * Pointer to the window-struct,
	 * which represents the created
	 * window itself.
	*/
	ENUD_Window *window;

	/*
	 * Pointer to the OpenGL-context,
	 * which is used for rendering
	 * both by the client and the
	 * ENUD-subsystem.
	*/
	ENUD_GLContext glcontext;

	/*
	 * Pointer to the UI-context
	 * created by the ENUD-subsystem.
	 * This struct is used to manage
	 * the userinterface.
	*/
	ENUD_UIContext *uicontext;

	/*
	 * This is just a shortcut to the
	 * root node in the node-tree of the
	 * previously declared ui-context.
	*/
	ENUD_Node *uiroot;

	/*
	 * Pointer to the camera, which is
	 * used to manage the current view.
	*/
	Camera *camera;

	/*
	 * Pointer to the world, which contains
	 * the tilemaps/heightmaps and the objects
	 * in the area of interest.
	*/
	World *world;

	/*
	 * Is the game currently running.
	*/
	int8_t running;

	/*
	 * Is fullscreen currently active.
	*/
	int8_t fullscr;

	/*
	 * This is the callback-function
	 * to handle user-inputs. This
	 * function will be set to NULL by
	 * default, but overwriting this
	 * variable will enable running
	 * this function whenever the user
	 * input something.
	*/
	void (*procevt)(ENUD_Event *e);

	/*
	 * This is the callback-function to
	 * handle the update-sequence. This
	 * function will be set to NULL by
	 * default, but overwriting this
	 * varaible will enable running this
	 * function on every update-cycle.
	*/
	void (*update)(void);

	/*
	 * This is the callback-function to
	 * handle the render-sequence. This
	 * function will be set to NULL by
	 * default, but overwriting this
	 * variable will enable running this
	 * function on every render-call.
	*/
	void (*render)(void);

	/*
	 * The absolute path to the directory
	 * the binary is in.
	*/
	char *bindir;
} gloWrapper;

extern uint8_t one;
extern uint8_t zero;

/* ===== THE ENUD-WINDOW ===== */
extern int g_win_flgs; 
extern int g_ren_flg;

/* ======= CORE-STRUCT ======= */
extern gloWrapper *core;

/* Create a new wrapper-object */
gloWrapper *initWrapper(void);

/* Delete a wrapper */
void delWrapper(gloWrapper *ptr);

#endif
