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


/* The updates-per-second */
#define GAME_HERTZ       60
#define TICK_TIME        (1000/GAME_HERTZ)
#define MAX_UPDATE_NUM   5

/* The shares-per-second */
#define SHARE_HERZ       30
#define SHARE_TIME       (1000/SHARE_HERZ)

#define LOC_OBJ_NUM      1
#define SHARE_BUF_SIZE   15

struct core_wrapper {
	char running;

	uint32_t last_update;
	uint32_t last_render;

	void (*proc_evt)(SDL_Event *evt);
	void (*update)(void);
	void (*render)(void);

	short obj[LOC_OBJ_NUM];


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
