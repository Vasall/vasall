#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "setup.h"
#include "global.h"
#include "handle.h"

/* ========= REDECLARE EXTERN VARIABLES ============== */
int g_win_flgs = XSDL_WINDOW_RESIZABLE | XSDL_WINDOW_OPENGL;

/*
 * Create and configure the basic settings of
 * the main window.
 *
 * Returns: Either a pointer to the new window
 * 	of NULL if an error occurred
*/
XSDL_Window *initWindow(void)
{
	XSDL_Window *win;

	win = XSDL_CreateWindow("Vasall",
			XSDL_WINDOWPOS_UNDEFINED,
			XSDL_WINDOWPOS_UNDEFINED,
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
	/* Create the XSDL-OpenGL-context */
	if((core->glcontext = XSDL_GL_CreateContext(core->window)) == NULL) {
		printf("Failed to create context.\n");
		return(-1);
	}

	/* Set clear-color */
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

	glEnable(GL_DEPTH_TEST);

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
	/* printf("Directory: %s\n", core->bindir);*/

	XSDL_CombinePath(path, core->bindir, "../res/fonts/mecha.ttf");
	if(XSDL_LoadFont(path, 24) < 0)
		goto loadfailed;

	XSDL_CombinePath(path, core->bindir, "../res/fonts/unifont.ttf");
	if(XSDL_LoadFont(path, 16) < 0)
		goto loadfailed;

	XSDL_CombinePath(path, core->bindir, "../res/fonts/aller.ttf");
	if(XSDL_LoadFont(path, 16) < 0)
		goto loadfailed;


	XSDL_CombinePath(path, core->bindir, "../res/fonts/editundo.ttf");
	if(XSDL_LoadFont(path, 48) < 0)
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
void try_login(XSDL_Node *n, XSDL_Event *e)
{
	if(n||e){/* Prevent wraning for not using paameters */}

	core->procevt = &game_procevt;
	core->update = &game_update;
	core->render = &game_render;

	/* Switch from menuscreen to gamescreen */
	XSDL_ModFlag(XSDL_Get(core->uiroot, "mns"), XSDL_FLG_ACT, &zero);
	XSDL_ModFlag(XSDL_Get(core->uiroot, "gms"), XSDL_FLG_ACT, &one);	

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
	XSDL_Rect body0 = {50, 14, 300, 52};
	XSDL_Rect body1 = {40, 96, 320, 24};
	XSDL_Rect body2 = {40, 176, 320, 24};
	XSDL_Color mns_text_col = { 0xF9, 0xF9, 0xF9, 0xFF };
	XSDL_Color mns_input_bck_col = {0x37, 0x37, 0x37, 0xFF};
	XSDL_Color mns_input_bor_col = {0x28, 0x28, 0x28, 0xFF};
	short mns_input_corners[] = {4, 4, 4, 4};
	XSDL_Color mns_form_bck_col = {0x3D, 0x3B, 0x3C, 0xff};
	short mns_form_corners[] = {5, 5, 5, 5};
	XSDL_Color mns_title_bck_col = {0xd3, 0x34, 0x5a, 0xff};
	short mns_title_cor[] = {5, 5, 0, 0};
	XSDL_Color gms_stats_bck_col = { 0x2b, 0x2b, 0x36, 0xf8};
	short gms_stats_cor[] = {3, 3, 3, 3};
	XSDL_Input *user_input;
	XSDL_Input *pswd_input;

	/* Create the menu-sceen */
	XSDL_CreateWrapper(core->uiroot, "mns", 0, 0, -100, -100);
	XSDL_CreateWrapper(XSDL_Get(core->uiroot, "mns"), "mns_form",
			-1, -1, 400, 380);

	XSDL_CreateWrapper(XSDL_Get(core->uiroot, "mns_form"), "mns_title",
			0, 0, 400, 80);
	XSDL_CreateText(XSDL_Get(core->uiroot, "mns_title"), "label0", &body0,
			"VASALL", &XSDL_WHITE, 3, 0);

	XSDL_CreateText(XSDL_Get(core->uiroot, "mns_form"), "label1", &body1,"Email:", 
			&XSDL_WHITE, 2, XSDL_TEXT_LEFT);
	XSDL_CreateInput(XSDL_Get(core->uiroot, "mns_form"), "mns_user", 40, 120, 320, 40, "");

	XSDL_CreateText(XSDL_Get(core->uiroot, "mns_form"), "label2", &body2, "Password:", 
			&XSDL_WHITE, 2, XSDL_TEXT_LEFT);
	XSDL_CreateInput(XSDL_Get(core->uiroot, "mns_form"), "mns_pswd", 40, 200, 320, 40, "");

	XSDL_CreateButton(XSDL_Get(core->uiroot, "mns_form"), "mns_login", 40, 270, 320, 
			40, "Login");

	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_user"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_user"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_user"), XSDL_STY_BCK_COL, &mns_input_bck_col);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_user"), XSDL_STY_BOR, &one);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_user"), XSDL_STY_BOR_COL, &mns_input_bor_col);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_user"), XSDL_STY_COR_RAD, &mns_input_corners);
	user_input = XSDL_Get(core->uiroot, "mns_user")->element;
	memcpy(&user_input->col, &mns_text_col, sizeof(XSDL_Color));

	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_pswd"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_pswd"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_pswd"), XSDL_STY_BCK_COL, &mns_input_bck_col);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_pswd"), XSDL_STY_BOR, &one);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_pswd"), XSDL_STY_BOR_COL, &mns_input_bor_col);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_pswd"), XSDL_STY_COR_RAD, &mns_input_corners);
	pswd_input = XSDL_Get(core->uiroot, "mns_pswd")->element;
	memcpy(&pswd_input->col, &mns_text_col, sizeof(XSDL_Color));

	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_form"), XSDL_STY_VIS, &one);
	/*XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_form"), XSDL_STY_BCK, &one);*/
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_form"), XSDL_STY_BCK_COL, &mns_form_bck_col);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_form"), XSDL_STY_COR_RAD, &mns_form_corners);

	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_title"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_title"), XSDL_STY_BCK_COL, &mns_title_bck_col);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "mns_title"), XSDL_STY_COR_RAD, &mns_title_cor);

	XSDL_BindEvent(XSDL_Get(core->uiroot, "mns_login"), XSDL_EVT_MOUSEDOWN, &try_login);

	/* Create the game-sceen */
	XSDL_CreateWrapper(core->uiroot, "gms", 0, 0, -100, -100);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "gms"), XSDL_STY_VIS, &one);

	XSDL_CreateWrapper(XSDL_Get(core->uiroot, "gms"), "gms_stats", -1, 5, 790, 35);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "gms_stats"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "gms_stats"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "gms_stats"), XSDL_STY_BCK_COL, &gms_stats_bck_col);
	XSDL_ModStyle(XSDL_Get(core->uiroot, "gms_stats"), XSDL_STY_COR_RAD, &gms_stats_cor);

	XSDL_ModFlag(XSDL_Get(core->uiroot, "gms"), XSDL_FLG_ACT, &zero);

	return(0);
}
