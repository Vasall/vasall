#ifndef _GAME_H
#define _GAME_H

#include "sdl.h"

extern short obj;
extern int running;

extern void game_proc_evt(SDL_Event *evt);
extern void game_update(void);
extern void game_render(void);

#endif
