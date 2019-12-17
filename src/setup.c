#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "setup.h"
#include "global.h"
#include "handle.h"

/* ========= REDECLARE EXTERN VARIABLES ============== */
int g_win_flgs = ENUD_WINDOW_RESIZABLE | ENUD_WINDOW_OPENGL;

/*
 * Create and configure the basic settings of
 * the main window.
 *
 * Returns: Either a pointer to the new window
 * 	of NULL if an error occurred
*/
ENUD_Window *initWindow(void)
{
	ENUD_Window *win;

	win = ENUD_CreateWindow("Vasall",
			ENUD_WINDOWPOS_UNDEFINED,
			ENUD_WINDOWPOS_UNDEFINED,
			800, 600,
			g_win_flgs);

	if(win == NULL) {
		return(NULL);
	}

	return(win);
}

/*
 * This function is going to setup
 * OpenGL with the default settings.
 *
 * Returns: 0 on success and -1 
 * 	if an error occurred
*/
int initGL(void)
{
	/* Create the ENUD-OpenGL-context */
	if((core->glcontext = ENUD_GL_CreateContext(core->window)) == NULL) {
		printf("Failed to create context.\n");
		return(-1);
	}

	/* Set clear-color */
	glClearColor(0.095, 0.095, 0.095, 1.0);



	return(0);
}

/*
 * Load all necessary resources, 
 * like fonts, sprites and more.
 *
 * Returns: 0 on success and -1 
 * 	if an error occurred
 */
int loadResources(void)
{
	char path[512];
	printf("Directory: %s\n", core->bindir);

	ENUD_CombinePath(path, core->bindir, "../res/fonts/mecha.ttf");
	if(ENUD_LoadFont(path, 24) < 0)
		goto loadfailed;

	ENUD_CombinePath(path, core->bindir, "../res/fonts/unifont.ttf");
	if(ENUD_LoadFont(path, 16) < 0)
		goto loadfailed;

	ENUD_CombinePath(path, core->bindir, "../res/fonts/aller.ttf");
	if(ENUD_LoadFont(path, 16) < 0)
		goto loadfailed;


	ENUD_CombinePath(path, core->bindir, "../res/fonts/editundo.ttf");
	if(ENUD_LoadFont(path, 48) < 0)
		goto loadfailed;

	return(0);

loadfailed:
	return(-1);
}

/*
 * This is a short callback-function to
 * switch from the menuscreen to the
 * gamescreen.
 *
 * @n: Pointer to the node this event is
 * 	attached to
 * @e: Pointer to the event
 */
void try_login(ENUD_Node *n, ENUD_Event *e)
{
	if(n||e){/* Prevent wraning for not using paameters */}

	core->procevt = &game_procevt;
	core->update = &game_update;
	core->render = &game_render;

	/* Switch from menuscreen to gamescreen */
	ENUD_ModFlag(ENUD_Get(core->uiroot, "mns"), ENUD_FLG_ACT, &zero);
	ENUD_ModFlag(ENUD_Get(core->uiroot, "gms"), ENUD_FLG_ACT, &one);	

	return;
}

/*
 * Initialize the UI and place all elements.
 *
 * Returns: 0 on success and -1
 * 	if an error occurred
 */
int initUI(void)
{
	ENUD_Rect body0 = {50, 14, 300, 52};
	ENUD_Rect body1 = {40, 96, 320, 24};
	ENUD_Rect body2 = {40, 176, 320, 24};
	ENUD_Color mns_text_col = { 0xF9, 0xF9, 0xF9, 0xFF };
	ENUD_Color mns_input_bck_col = {0x37, 0x37, 0x37, 0xFF};
	ENUD_Color mns_input_bor_col = {0x28, 0x28, 0x28, 0xFF};
	short mns_input_corners[] = {4, 4, 4, 4};
	ENUD_Color mns_form_bck_col = {0x3D, 0x3B, 0x3C, 0xff};
	short mns_form_corners[] = {5, 5, 5, 5};
	ENUD_Color mns_title_bck_col = {0xd3, 0x34, 0x5a, 0xff};
	short mns_title_cor[] = {5, 5, 0, 0};
	ENUD_Color gms_stats_bck_col = { 0x2b, 0x2b, 0x36, 0xf8};
	short gms_stats_cor[] = {3, 3, 3, 3};
	ENUD_Input *user_input;
	ENUD_Input *pswd_input;

	/* Create the menu-sceen */
	ENUD_CreateWrapper(core->uiroot, "mns", 0, 0, -100, -100);
	ENUD_CreateWrapper(ENUD_Get(core->uiroot, "mns"), "mns_form",
			-1, -1, 400, 380);

	ENUD_CreateWrapper(ENUD_Get(core->uiroot, "mns_form"), "mns_title",
			0, 0, 400, 80);
	ENUD_CreateText(ENUD_Get(core->uiroot, "mns_title"), "label0", &body0,
			"VASALL", &ENUD_WHITE, 3, 0);

	ENUD_CreateText(ENUD_Get(core->uiroot, "mns_form"), "label1", &body1,"Email:", 
			&ENUD_WHITE, 2, ENUD_TEXT_LEFT);
	ENUD_CreateInput(ENUD_Get(core->uiroot, "mns_form"), "mns_user", 40, 120, 320, 40, "");

	ENUD_CreateText(ENUD_Get(core->uiroot, "mns_form"), "label2", &body2, "Password:", 
			&ENUD_WHITE, 2, ENUD_TEXT_LEFT);
	ENUD_CreateInput(ENUD_Get(core->uiroot, "mns_form"), "mns_pswd", 40, 200, 320, 40, "");

	ENUD_CreateButton(ENUD_Get(core->uiroot, "mns_form"), "mns_login", 40, 270, 320, 
			40, "Login");

	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_user"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_user"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_user"), ENUD_STY_BCK_COL, &mns_input_bck_col);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_user"), ENUD_STY_BOR, &one);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_user"), ENUD_STY_BOR_COL, &mns_input_bor_col);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_user"), ENUD_STY_COR_RAD, &mns_input_corners);
	user_input = ENUD_Get(core->uiroot, "mns_user")->element;
	memcpy(&user_input->col, &mns_text_col, sizeof(ENUD_Color));

	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_pswd"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_pswd"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_pswd"), ENUD_STY_BCK_COL, &mns_input_bck_col);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_pswd"), ENUD_STY_BOR, &one);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_pswd"), ENUD_STY_BOR_COL, &mns_input_bor_col);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_pswd"), ENUD_STY_COR_RAD, &mns_input_corners);
	pswd_input = ENUD_Get(core->uiroot, "mns_pswd")->element;
	memcpy(&pswd_input->col, &mns_text_col, sizeof(ENUD_Color));

	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_form"), ENUD_STY_VIS, &one);
	/*ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_form"), ENUD_STY_BCK, &one);*/
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_form"), ENUD_STY_BCK_COL, &mns_form_bck_col);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_form"), ENUD_STY_COR_RAD, &mns_form_corners);

	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_title"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_title"), ENUD_STY_BCK_COL, &mns_title_bck_col);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "mns_title"), ENUD_STY_COR_RAD, &mns_title_cor);

	ENUD_BindEvent(ENUD_Get(core->uiroot, "mns_login"), ENUD_EVT_MOUSEDOWN, &try_login);

	/* Create the game-sceen */
	ENUD_CreateWrapper(core->uiroot, "gms", 0, 0, -100, -100);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "gms"), ENUD_STY_VIS, &one);

	ENUD_CreateWrapper(ENUD_Get(core->uiroot, "gms"), "gms_stats", -1, 5, 790, 35);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "gms_stats"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "gms_stats"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "gms_stats"), ENUD_STY_BCK_COL, &gms_stats_bck_col);
	ENUD_ModStyle(ENUD_Get(core->uiroot, "gms_stats"), ENUD_STY_COR_RAD, &gms_stats_cor);

	ENUD_ModFlag(ENUD_Get(core->uiroot, "gms"), ENUD_FLG_ACT, &zero);

	return(0);
}
