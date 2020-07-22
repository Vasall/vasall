#ifndef _UPDATE_H
#define _UPDATE_H

#include "core.h"

/* Basic update-functions */
extern void game_start(void);
extern void game_proc_evt(SDL_Event *evt);
extern void game_update(void);
extern void game_render(void);

#endif
