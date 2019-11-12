/* Using SDL and standard IO */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <unistd.h>

#include "../ENUD/enud.h"
#include "vector.h"

uint8_t one = 1;
uint8_t zero = 0;

/* =============== WINDOW SETTINGS =================== */

int *win_w;
int *win_h;
int win_flgs = SDL_WINDOW_RESIZABLE; 
int ren_flg = ENUD_RENDERER_ACCELERATED | ENUD_RENDERER_PRESENTVSYNC;
ENUD_Color win_clr = { 0x18, 0x18, 0x18};

/* ============== GLOBAL VARIABLES =================== */

uint8_t running = 0;
uint8_t fullscr = 0;
ENUD_Window *window = NULL;
ENUD_Renderer *renderer = NULL;
ENUD_UIContext *context = NULL;
ENUD_Node *root = NULL;
ENUD_Event event;

/* ================= PROTOTYPES ====================== */

/* Initialize the game */
ENUD_Window *init_window();
ENUD_Renderer *init_renderer(ENUD_Window *win);
int init_resources();
void init_gui(ENUD_UIContext *ctx);

void toggle_fullscreen();
void try_login();

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

	if((window = init_window()) == NULL) {
		printf("[!] Window could not be created! (%s)\n", ENUD_GetError());
		goto cleanup_sdl;
	}

	if((renderer = init_renderer(window)) == NULL) {
		printf("[!] Renderer could not be created! (%s)\n", ENUD_GetError());
		goto cleanup_window;
	}

	if((context = ENUD_CreateUIContext(window)) == NULL) {
		printf("[!] Context could not be created!\n");
		goto cleanup_renderer;
	}
	root = context->root;

	if(init_resources(argv[0]) < 0) {
		printf("[!] Couldn't load all resources!\n");
		goto cleanup_renderer;
	}

	/* Initialize the user-interface */
	init_gui(context);

	/* Mark game as running */
	running = 1;

	/* Run the game */
	while(running) {
		/* -------- EVENTS -------- */
		/* Process user-input */
		while(ENUD_PollEvent(&event)) {
			if(event.type == ENUD_QUIT)
				running = 0;

			/* Process interactions with the UI */
			if(ENUD_ProcEvent(context, &event) > -1)
				continue;
		}

		/* -------- UPDATE -------- */


		/* -------- RENDER -------- */
		/* Clear the screen */
		ENUD_SetRenderDrawColor(renderer, &win_clr);
		ENUD_RenderClear(renderer);

		/* Update UI-nodes */
		ENUD_Update(context);

		/* Render visible UI-elements */
		ENUD_RenderPipe(renderer, context->pipe);

		/* Render all elements in the active scene */
		ENUD_RenderPresent(renderer);
	}

	/* Destroy context */
	ENUD_DeleteUIContext(context);

cleanup_renderer:
	/* Destory renderer */
	ENUD_DestroyRenderer(renderer);
cleanup_window:
	/* Destroy window */
	ENUD_DestroyWindow(window);
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
			win_flgs);

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
	ENUD_Renderer *ren = ENUD_CreateRenderer(win, -1, ren_flg);

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

	sprintf(path, "%s/%s", exe_dir, "res/aller.ttf");
	if(ENUD_LoadFont(path, 16) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/editundo.ttf");
	if(ENUD_LoadFont(path, 48) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/mns_bck.png");
	if(ENUD_LoadImage(renderer, path) < 0)
		goto loadfailed;

	return(0);

loadfailed:
	return(-1);
}

/*
 * Initialize the GUI and place all elements.
 *
 * @ctx: The context to create
 */
void init_gui(ENUD_UIContext *ctx)
{
	ENUD_Node *rootnode = ctx->root;

	/* Create the menu-sceen */
	ENUD_CreateWrapper(rootnode, "mns", 0, 0, -100, -100);
	ENUD_CreateWrapper(ENUD_Get(rootnode, "mns"), "mns_form",
			-1, -1, 400, 380);

	ENUD_CreateWrapper(ENUD_Get(rootnode, "mns_form"), "mns_title",
			0, 0, 400, 80);
	ENUD_Rect body0 = {50, 14, 300, 52};
	ENUD_CreateText(ENUD_Get(rootnode, "mns_title"), "label0", &body0,
			"VASALL", &ENUD_WHITE, 2, 0);

	ENUD_Rect body1 = {40, 106, 320, 24};
	ENUD_CreateText(ENUD_Get(rootnode, "mns_form"), "label1", &body1,"Email:", 
			&ENUD_WHITE, 1, ENUD_TEXT_LEFT);
	ENUD_CreateInput(ENUD_Get(rootnode, "mns_form"), "mns_user", 40, 130, 320, 40, "");

	ENUD_Rect body2 = {40, 186, 320, 24};
	ENUD_CreateText(ENUD_Get(rootnode, "mns_form"), "label2", &body2, "Password:", 
			&ENUD_WHITE, 1, ENUD_TEXT_LEFT);
	ENUD_CreateInput(ENUD_Get(rootnode, "mns_form"), "mns_pswd", 40, 210, 320, 40, "");

	ENUD_CreateButton(ENUD_Get(rootnode, "mns_form"), "mns_login", 40, 280, 320, 40, "Login");

	ENUD_Color mns_form_bck_col = {0x23, 0x23, 0x23, 0xff};
	short mns_form_corners[] = {5, 5, 5, 5};
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_form"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_form"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_form"), ENUD_STY_BCK_COL, &mns_form_bck_col);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_form"), ENUD_STY_COR_RAD, &mns_form_corners);

	ENUD_Color mns_title_bck_col = {0xd3, 0x34, 0x5a, 0xff};
	short mns_title_cor[] = {5, 5, 0, 0};
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_title"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_title"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_title"), ENUD_STY_BCK_COL, &mns_title_bck_col);
	ENUD_ModStyle(ENUD_Get(rootnode, "mns_title"), ENUD_STY_COR_RAD, &mns_title_cor);

	ENUD_BindEvent(ENUD_Get(rootnode, "mns_login"), ENUD_EVT_MOUSEDOWN, &try_login);

	/* Create the game-sceen */
	ENUD_CreateWrapper(rootnode, "gms", 0, 0, -100, -100);
	ENUD_Color gms_bck_col = {0x47, 0x2d, 0x5c, 0xff};
	ENUD_ModStyle(ENUD_Get(rootnode, "gms"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "gms"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "gms"), ENUD_STY_BCK_COL, &gms_bck_col);

	ENUD_CreateWrapper(ENUD_Get(rootnode, "gms"), "gms_stats", -1, 5, 780, 35);
	ENUD_Color gms_stats_bck_col = {0x23, 0x25, 0x30, 0xff};
	short gms_stats_cor[] = {6, 6, 6, 6};
	ENUD_ModStyle(ENUD_Get(rootnode, "gms_stats"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "gms_stats"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(rootnode, "gms_stats"), ENUD_STY_BCK_COL, &gms_stats_bck_col);
	ENUD_ModStyle(ENUD_Get(rootnode, "gms_stats"), ENUD_STY_COR_RAD, &gms_stats_cor);

	ENUD_ModFlag(ENUD_Get(rootnode, "gms"), ENUD_FLG_ACT, &zero);
}

void toggle_fullscreen()
{
	fullscr = !fullscr;

	if(fullscr)
		ENUD_SetWindowFullscreen(window, win_flgs | 
				ENUD_WINDOW_FULLSCREEN_DESKTOP);
	else
		ENUD_SetWindowFullscreen(window, win_flgs);
}

void try_login()
{
	printf("Trying to connect to server...\n");

	ENUD_ModFlag(ENUD_Get(root, "mns"), ENUD_FLG_ACT, &zero);
	ENUD_ModFlag(ENUD_Get(root, "gms"), ENUD_FLG_ACT, &one);

	return;
}
