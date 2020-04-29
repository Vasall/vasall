#ifndef _V_HANDLE_H_
#define _V_HANDLE_H_

#include "defines.h"
#include "core.h"

/* Handle the resize-event */
V_API void handle_resize(XSDL_Event *evt);

/* MENUSCREEN::CALLBACKS */
V_API void menu_procevt(XSDL_Event *evt); 
V_API void menu_update(void);
V_API void menu_render(void);

/* GAMESCREEN::CALLBACKS */
V_API void game_procevt(XSDL_Event *evt);
V_API void game_update(void);
V_API void game_render(void);

#endif
