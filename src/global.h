#ifndef VASALL_GLOBAL_H
#define VASALL_GLOBAL_H

#include "../ENUD/enud.h"
#include "camera.h"

static uint8_t one = 1;
static uint8_t zero = 0;

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

/* ====== RUNTIME-FLAGS ====== */
extern uint8_t g_running;
extern uint8_t g_fullscr;

/* ==== RUNTIME-FUNCTIONS ==== */
extern void (*g_procevt)();
extern void (*g_update)();

/* ====== RENDER STRUCTS ===== */
extern vsCamera g_camera;

#endif
