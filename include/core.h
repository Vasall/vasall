#ifndef _CORE_H  
#define _CORE_H

#include "network.h"
#include "sdl.h"
#include "window.h"
#include "node.h"
#include "stdnode.h"
#include "asset.h"
#include "model.h"
#include "object.h"
#include "camera.h"
#include "world.h"

struct core_wrapper {
	char running;

	void (*proc_evt)(SDL_Event *evt);
	void (*update)(void);
	void (*render)(void);

	short obj;
};


/* The global core-wrapper-instance */
extern struct core_wrapper core;


/*
 * Initialize the global-core wrapper and setup the necessary values.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int core_init(void);


/*
 * Gather and process the general events.
 */
extern void core_proc_evt(void);


/*
 * Update the game and call the custom update-function.
 */
extern void core_update(void);


/*
 * Render the game and call the custom render-function.
 */
extern void core_render(void);

#endif
