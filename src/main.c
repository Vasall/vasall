/* Using SDL and standard IO */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <unistd.h>

#define ENUD_IMPLEMENTATION
#define ENUD_DEFINE_LIB
#define ENUD_DEFINE_ONCE
#include "../ENUD/enud.h"

#define VASALL_DEFINE_GLOBAL
#include "global.h"

#include "vector.h"
#include "gui.h"
#include "world.h"
#include "update.h"

/* =============== WINDOW SETTINGS =================== */

int g_win_flgs = 0; 
int g_ren_flg = ENUD_RENDERER_ACCELERATED | ENUD_RENDERER_PRESENTVSYNC;
ENUD_Color g_win_clr = {0x18, 0x18, 0x18, 0xFF };

/* ============== GLOBAL VARIABLES =================== */

uint8_t g_running = 0;
uint8_t g_fullscr = 0;
ENUD_Window *g_window = NULL;
ENUD_Renderer *g_renderer = NULL;
ENUD_UIContext *g_context = NULL;
ENUD_Node *g_root = NULL;

vsCamera g_camera = {
	{10, 10},
	1.0,
	32
};

vsWorld *g_world = NULL;

ENUD_Event event;

void (*g_procevt)() = NULL;
void (*g_update)() = NULL;

/* ================= PROTOTYPES ====================== */

/* Initialize the game */
ENUD_Window *init_window();
ENUD_Renderer *init_renderer(ENUD_Window *win);
int init_resources();

void toggle_fullscreen();

/* ================ MAIN FUNCTION ==================== */

int main(int argc, char **argv) 
{
	printf("\nStarting vasall-client...\n");

	if(ENUD_Init(ENUD_INIT_EVERYTHING) < 0) {
		printf("[!] SDL could not initialize! (%s)\n", ENUD_GetError());
		goto exit;
	}
	ENUD_ShowVersions();
	printf("\n");

	if((g_window = init_window()) == NULL) {
		printf("[!] Window could not be created! (%s)\n", ENUD_GetError());
		goto cleanup_sdl;
	}

	if((g_renderer = init_renderer(g_window)) == NULL) {
		printf("[!] Renderer could not be created! (%s)\n", ENUD_GetError());
		goto cleanup_window;
	}

	if((g_context = ENUD_CreateUIContext(g_window)) == NULL) {
		printf("[!] Context could not be created!\n");
		goto cleanup_renderer;
	}
	g_root = g_context->root;

	if(init_resources(argv[0]) < 0) {
		printf("[!] Couldn't load all resources!\n");
		goto cleanup_renderer;
	}

	/* Initialize the user-interface */
	init_gui();

	g_world = wld_create(1024, 1024, 3);
	
	/* Mark game as running */
	g_running = 1;

	g_procevt = &menu_procevt;
	g_update = &menu_update;

	/* Run the game */
	while(g_running) {
		/* Clear the screen */
		ENUD_SetRenderDrawColor(g_renderer, &g_win_clr);
		ENUD_RenderClear(g_renderer);

		/* -------- EVENTS -------- */
		/* Process user-input */
		while(ENUD_PollEvent(&event)) {
			if(event.type == ENUD_QUIT)
				g_running = 0;

			/* Process interactions with the UI */
			if(ENUD_ProcEvent(g_context, &event) > -1)
				continue;

			/* Process interacttion with the game */
			if(g_procevt != NULL) {
				g_procevt(&event);
			}
		}

		/* -------- UPDATE -------- */
		if(g_update != NULL) {
			g_update();
		}

		/* Update UI-nodes */
		ENUD_UpdateUIContext(g_context);

		/* Render visible UI-elements */
		ENUD_RenderPipe(g_renderer, g_context->pipe);

		/* Render all elements in the active scene */
		ENUD_RenderPresent(g_renderer);
	}

	/* Destroy context */
	ENUD_DeleteUIContext(g_context);

cleanup_renderer:
	/* Destory renderer */
	ENUD_DestroyRenderer(g_renderer);
cleanup_window:
	/* Destroy window */
	ENUD_DestroyWindow(g_window);
cleanup_sdl:
	/* Quit ENUD subsystems */
	ENUD_Quit();
exit:
	return (0);
}

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */
/*                    SETUP FUNCTIONS                  */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

/* ============= INITIALIZE THE GAME ================= */

/*
 * Initialize the window and configure
 * basic settings like the title, minimal
 * window size and window-icon.
 *
 * Returns: Window-Pointer or NULL if an error occurred
 */
ENUD_Window *init_window()
{
	ENUD_Window *win = ENUD_CreateWindow(
			"Vasall", 
			ENUD_WINDOWPOS_UNDEFINED, 
			ENUD_WINDOWPOS_UNDEFINED, 
			800, 600, 
			g_win_flgs);

	if(win == NULL) 
		return (NULL);

	ENUD_SetWindowIcon(win);

	return(win);
}

/*
 * This function will initialize the SDL-renderer
 * and set the render-flags. Note that it's very important
 * to enable VSync, or otherwise the either the Framerate
 * or performance will be off.
 *
 * @win: Pointer to underlying window
 *
 * Returns: Renderer-pointer or NULL if an error occurred
 */
ENUD_Renderer *init_renderer(ENUD_Window *win)
{
	ENUD_Renderer *ren = ENUD_CreateRenderer(win, -1, g_ren_flg);

	return(ren);
}

/*
 * Load all necessary resources, like fonts, sprites
 * and more.
 *
 * Returns: 0 on success and -1 if an error occurred
 */
int init_resources()
{
	char cwd[256];
	char exe_dir[256];
	char path[512];
	int i = 0;

	readlink("/proc/self/exe", cwd, 256);
	for(i = strlen(cwd); i >= 0; i--)
		if(cwd[i] == '/')
			break;
	memcpy(exe_dir, cwd, i);

	printf("Directory: %s\n", exe_dir);

	sprintf(path, "%s/%s", exe_dir, "res/mecha.ttf");	
	if(ENUD_LoadFont(path, 24) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/unifont.ttf");
	if(ENUD_LoadFont(path, 16) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/aller.ttf");
	if(ENUD_LoadFont(path, 16) < 0)
		goto loadfailed;


	sprintf(path, "%s/%s", exe_dir, "res/editundo.ttf");
	if(ENUD_LoadFont(path, 48) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/mns_bck.png");
	if(ENUD_LoadImage(g_renderer, path) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/sprites/house_00.png");
	if(ENUD_LoadImage(g_renderer, path) < 0)
		goto loadfailed;

	return(0);

loadfailed:
	return(-1);
}

void toggle_fullscreen()
{
	g_fullscr = !g_fullscr;

	if(g_fullscr)
		ENUD_SetWindowFullscreen(g_window, g_win_flgs | 
				ENUD_WINDOW_FULLSCREEN_DESKTOP);
	else
		ENUD_SetWindowFullscreen(g_window, g_win_flgs);
}
