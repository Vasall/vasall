#ifndef _HANDLE_H_
#define _HANDLE_H_

#include "global.h"

/* Handle the resize-event */
void handle_resize(XSDL_Event *evt);

/* MENUSCREEN::CALLBACKS */
void menu_procevt(XSDL_Event *evt); 
void menu_update(void);
void menu_render(void);

/* GAMESCREEN::CALLBACKS */
void game_procevt(XSDL_Event *evt);
void game_update(void);
void game_render(void);

#endif
