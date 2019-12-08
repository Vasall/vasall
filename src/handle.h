#ifndef VASALL_HANDLE_H
#define VASALL_HANDLE_H

#include "wrapper.h"

/* MENUSCREEN::CALLBACKS */
void menu_procevt(ENUD_Event *evt); 
void menu_update(void);
void menu_render(void);

/* GAMESCREEN::CALLBACKS */
void game_procevt(ENUD_Event *evt);
void game_update(void);
void game_render(void);

#endif
