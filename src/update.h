#ifndef VASALL_UPDATE_H
#define VASALL_UPDATE_H

#include <SDL2/SDL.h>
#include "../ENUD/enud.h"
#include "global.h"

void menu_procevt(ENUD_Event *evt);
void menu_update();

void game_procevt(ENUD_Event *evt);
void game_update();

#endif
