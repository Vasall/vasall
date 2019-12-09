#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "setup.h"
#include "wrapper.h"
#include "handle.h"

/* ========= REDECLARE EXTERN VARIABLES ============== */
int g_win_flgs = ENUD_WINDOW_RESIZABLE | ENUD_WINDOW_OPENGL;
int g_ren_flg = ENUD_RENDERER_ACCELERATED | ENUD_RENDERER_PRESENTVSYNC;


float lightPos[4] = {0,65,0, 1};

/*
 * Initialize the frameworks used by the client
 * and setup the main structs for the camera,
 * world, etc.
 *
 * @argc: The amount of arguments passed to main
 * @argv: A buffer containing all arguments passed
 * 	to main
 *
 * Returns: 0 on sucess and -1 if an error occurred
*/
int init(int argc, char **argv)
{
	if(argc < 1) {
		return(-1);
	}

	printf("Init ENUD-subsystem.\n");
	if(ENUD_Init(ENUD_INIT_EVERYTHING) < 0) {                                                    
		printf("[!] ENUD could not initialize! (%s)\n", ENUD_GetError());
		return(-1);
	}

	printf("Init core-wrapper.\n");
	if((core = initWrapper()) == NULL) {
		printf("Failed to initialize core-wrapper.\n");
		goto cleanup_enud;
	}

	printf("Init window.\n");
	if((core->window = initWindow()) == NULL) {
		printf("[!] Failed to create window! (%s)\n", ENUD_GetError());
		goto cleanup_enud;
	}

	printf("Init OpenGL.\n");
	if(initGL() < 0) {
		printf("[!] Failed to initialize OpenGL.\n");
		goto cleanup_enud;
	}

	printf("Init UI-context.\n");
	if((core->uicontext = ENUD_CreateUIContext(core->window)) == NULL) {
		printf("[!] Failed to setup ui-context.\n");
		goto cleanup_enud;
	}
	core->uiroot = core->uicontext->root;

	core->bindir = ENUD_GetBinDir(argv[0]);

	printf("Load resources.\n");
	if(loadResources() < 0) {
		printf("[!] Failed to load resources.\n");
		goto cleanup_enud;
	}

	printf("Init UI-elements.\n");
	if(initUI() < 0) {
		printf("[!] Failed to setup ui.\n");
		goto cleanup_enud;
	}

	printf("Init camera.\n");
	if((core->camera = initCamera()) == NULL) {
		printf("[!] Failed ot setup camera.\n");
		goto cleanup_enud;
	}

	printf("Init world-container.\n");
	if((core->world = initWorld()) == NULL) {
		printf("[!] Failed to initialize world.\n");
		goto cleanup_enud;
	}

	return(0);

cleanup_enud:
	ENUD_Quit();

	return(-1);
}

/*
 * This function is going to setup
 * OpenGL with the default settings.
 *
 * Returns: 0 on sucess and -1 if an error occurred
*/
int initGL(void)
{
	int ratio;

	/* Create the ENUD-OpenGL-context */
	if((core->glcontext = ENUD_GL_CreateContext(core->window)) == NULL) {
		return(-1);
	}

	/* Enable flat shading (for artistic reasons) */
	glShadeModel(GL_FLAT);

	/* Set clear-color */
	glClearColor(0.095, 0.095, 0.095, 1.0);
	glEnable(GL_DEPTH_TEST);

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	/* Set projection matrix, using perspective w/ correct aspect ratio */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();  

	/* Enable flat shading (for artistic reasons) */
	glShadeModel(GL_FLAT);

	/* Set clear-color */
	glClearColor(0.095, 0.095, 0.095, 1);
	/*glEnable(GL_DEPTH_TEST);*/

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	/* Set projection matrix, using perspective w/ correct aspect ratio */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/* Create the camera-view */
	ratio = 800 / 600;
	glOrtho(-50 * ratio, 50 * ratio, -50, 50, -200, 200);	

	glViewport(0, 0, 800, 600);

	return(0);
}

/*
 * Create and configure the basic settings of
 * the main window.
 *
 * Returns: Either a pointer to the new window
 * 	of NULL if an error occurred
*/
ENUD_Window *initWindow(void)
{
	ENUD_Window *win = ENUD_CreateWindow("Vasall",
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
 * Load all necessary resources, like fonts, sprites
 * and more.
 *
 * Returns: 0 on success and -1 if an error occurred
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

static void try_login(ENUD_Node *n, ENUD_Event *e)
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
