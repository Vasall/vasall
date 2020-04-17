#ifndef _CORE_H
#define _CORE_H

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include "XSDL/XSDL.h"
#include "XSDL/XSDL_cache.h"
#include "XSDL/XSDL_context.h"
#include "XSDL/XSDL_img.h"
#include "XSDL/XSDL_init.h"
#include "XSDL/XSDL_input.h"
#include "XSDL/XSDL_list.h"
#include "XSDL/XSDL_node.h"
#include "XSDL/XSDL_render.h"
#include "XSDL/XSDL_stdnodes.h"
#include "XSDL/XSDL_struct.h"
#include "XSDL/XSDL_text.h"
#include "XSDL/XSDL_utf8.h"
#include "XSDL/XSDL_utils.h"

#include "vec.h"
#include "mat.h"
#include "list.h"
#include "hashtable.h"
#include "camera.h"
#include "model.h"
#include "model_utils.h"
#include "object.h"
#include "world.h"
#include "thing.h"

/*
 * This is a struct used as a global-wrapper.
 * This has the purpose of making the most
 * important objects and pointers accessable
 * throughout the whole project. The main
 * wrapper should be called core.
 */
struct core_wrapper {
	/*
	 * Pointer to the window-struct,
	 * which represents the created
	 * window itself.
	*/
	XSDL_Window *window;

	/*
	 * Pointer to the OpenGL-context,
	 * which is used for rendering
	 * both by the client and the
	 * XSDL-subsystem.
	*/
	XSDL_GLContext glcontext;

	/*
	 * Pointer to the UI-context
	 * created by the XSDL-subsystem.
	 * This struct is used to manage
	 * the userinterface.
	*/
	XSDL_UIContext *uicontext;

	/*
	 * This is just a shortcut to the
	 * root node in the node-tree of the
	 * previously declared ui-context.
	*/
	XSDL_Node *uiroot;

	/*
	 * The absolute path to the directory
	 * the binary is in.
	*/
	char *bindir;

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
	void (*procevt)(XSDL_Event *e);

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

	struct chunk_manager *chunks;

	struct object *obj;
};

extern uint8_t one;
extern uint8_t zero;
extern int g_win_flgs;
extern struct core_wrapper *core;
extern char glo_err_buf[256];

/* 
 * Set a new error-message.
 *
 * @err: The string to set as the new
 * 	error message
*/
void glo_set_err(char *err);

/* 
 * Get the most recent error-message.
 *
 * Returns: The most recent error-message
*/
char *glo_get_err(void);

/*
 * Initialize the core-wrapper and setup the
 * module instances and prepare the attributes.
 *
 * @argc: The number of arguments passed to main
 * @argv: The argument-buffer passed to main
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
*/
int core_init(int argc, char **argv);

/*
 * Destroy a global-wrapper and free the
 * allocated space.
*/
void core_close(void);

/* 
 * Read the include-register and import all necessary
 * resources, which should be defined in the file.
 *
 * @pth: The path to the include-register
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int core_load(char *pth);

#endif
