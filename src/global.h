#ifndef VASALL_GLOBAL_H
#define VASALL_GLOBAL_H

#include "../ENUD/enud.h"
#include "camera.h"
#include "world.h"

extern uint8_t one;
extern uint8_t zero;

/* ===== THE ENUD-WINDOW ===== */
extern int g_win_flgs; 
extern ENUD_Color g_win_clr;
extern ENUD_Window *g_window;

/* ==== THE ENUD-RENDERER ==== */
extern int g_ren_flg;
extern ENUD_Renderer *g_renderer;

/* ======= THE ENUD-UI ======= */
extern ENUD_UIContext *g_context;
extern ENUD_Node *g_root;

/* ======= GAME STRUCTS ====== */
#ifndef NOT_DEFINE_CAMERA
extern vsCamera g_camera;
#endif

#ifndef NOT_DEFINE_WORLD
extern vsWorld *g_world;
#endif

/* ====== RUNTIME-FLAGS ====== */
extern uint8_t g_running;
extern uint8_t g_fullscr;

/* ==== RUNTIME-FUNCTIONS ==== */
extern void (*g_procevt)();
extern void (*g_update)();

#ifdef VASALL_DEFINE_GLOBAL
uint8_t zero = 0;
uint8_t one = 1;
#endif


#endif
