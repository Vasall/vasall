#ifndef _GAME_H
#define _GAME_H

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


extern struct core_wrapper core;


extern int core_init(void);

extern void core_proc_evt(void);
extern void core_update(void);
extern void core_render(void);

#endif
