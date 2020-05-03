#include "setup.h"
#include "core.h"
#include "handle.h"
#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Load all necessary resources, 
 * like fonts, sprites and more.
 *
 * Returns: 0 on success and -1 
 * 	if an error occurred
 */
V_API int load_res(void)
{
	char path[512];
	/* printf("Directory: %s\n", core->bindir);*/

	XSDL_CombinePath(path, core->bindir, "../res/fonts/mecha.ttf");
	if(XSDL_LoadFont(path, 24) < 0) goto loadfailed;

	XSDL_CombinePath(path, core->bindir, "../res/fonts/unifont.ttf");
	if(XSDL_LoadFont(path, 16) < 0) goto loadfailed;

	XSDL_CombinePath(path, core->bindir, "../res/fonts/aller.ttf");
	if(XSDL_LoadFont(path, 16) < 0) goto loadfailed;

	XSDL_CombinePath(path, core->bindir, "../res/fonts/editundo.ttf");
	if(XSDL_LoadFont(path, 48) < 0) goto loadfailed;

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
V_INTERN void try_login(XSDL_Node *n, XSDL_Event *e)
{
	struct XSDL_Input *uname_node, *pswd_node;
	char uname[17];
	char pswd[32];

	if(n||e){/* Prevent warning for not using paameters */}	

	if(1) {
		struct XSDL_Node *node;

		/* Update core functions */
		core->procevt = &game_procevt;
		core->update = &game_update;
		core->render = &game_render;

		/* Switch from menuscreen to gamescreen */
		node = XSDL_Get(window->ui_root, "mns");
		XSDL_ModFlag(node, XSDL_FLG_ACT, &zero);
		return;
	}

	uname_node = XSDL_Get(window->ui_root, "mns_user")->element;
	pswd_node = XSDL_Get(window->ui_root, "mns_pswd")->element; 

	strcpy(uname, uname_node->buffer);
	strcpy(pswd, pswd_node->buffer);

	if(cli_sto(uname, pswd) == 1) {
		struct XSDL_Node *node;

		printf("Successfully logged into the system!\n");

		/* Update core functions */
		core->procevt = &game_procevt;
		core->update = &game_update;
		core->render = &game_render;

		/* Switch from menuscreen to gamescreen */
		node = XSDL_Get(window->ui_root, "mns");
		XSDL_ModFlag(node, XSDL_FLG_ACT, &zero);
		return;
	}

	printf("Failed to log into the system!\n");
}

/*
 * Initialize the UI and place all elements.
 *
 * Returns: 0 on success and -1
 * 	if an error occurred
 */
V_API int init_ui(void)
{
	XSDL_Rect body0 = {50, 14, 300, 52};
	XSDL_Rect body1 = {40, 96, 320, 24};
	XSDL_Rect body2 = {40, 176, 320, 24};
	XSDL_Color mns_text_col = { 0xF9, 0xF9, 0xF9, 0xFF };
	XSDL_Color mns_input_bck_col = {0x37, 0x37, 0x37, 0xFF};
	XSDL_Color mns_input_bor_col = {0x28, 0x28, 0x28, 0xFF};
	XSDL_Color mns_form_bck_col = {0x3D, 0x3B, 0x3C, 0xfb};
	short mns_form_corners[] = {8, 8, 8, 8};
	XSDL_Color mns_title_bck_col = {0xd3, 0x34, 0x5a, 0xff};
	XSDL_Color mns_login_bck_col = {0xff, 0x00, 0x00, 0xff};
	short mns_title_cor[] = {5, 5, 0, 0};
	XSDL_Input *user_input;
	XSDL_Input *pswd_input;

	/* Create the menu-sceen */
	XSDL_CreateWrapper(window->ui_root, "mns", 0, 0, 800, 600);
	XSDL_EnableNodeTex(XSDL_Get(window->ui_root, "mns"));
	XSDL_CreateWrapper(XSDL_Get(window->ui_root, "mns"), "mns_form", 200, 80, 400, 380);

	XSDL_CreateWrapper(XSDL_Get(window->ui_root, "mns_form"), "mns_title", 0, 0, 400, 80);
	XSDL_CreateText(XSDL_Get(window->ui_root, "mns_title"), "label0", &body0, "VASALL", 
			&XSDL_WHITE, 3, 0);

	XSDL_CreateText(XSDL_Get(window->ui_root, "mns_form"), "label1", &body1,"Email:", 
			&XSDL_WHITE, 2, XSDL_TEXT_LEFT);
	XSDL_CreateInput(XSDL_Get(window->ui_root, "mns_form"), "mns_user", 40, 120, 320, 40, "");

	XSDL_CreateText(XSDL_Get(window->ui_root, "mns_form"), "label2", &body2, "Password:", 
			&XSDL_WHITE, 2, XSDL_TEXT_LEFT);
	XSDL_CreateInput(XSDL_Get(window->ui_root, "mns_form"), "mns_pswd", 40, 200, 320, 40, "");

	XSDL_CreateButton(XSDL_Get(window->ui_root, "mns_form"), "mns_login", 40, 270, 320, 40, 
			"Login");

	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_user"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_user"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_user"), XSDL_STY_BCK_COL, &mns_input_bck_col);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_user"), XSDL_STY_BOR, &one);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_user"), XSDL_STY_BOR_COL, &mns_input_bor_col);
	user_input = XSDL_Get(window->ui_root, "mns_user")->element;
	memcpy(&user_input->col, &mns_text_col, sizeof(XSDL_Color));

	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_pswd"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_pswd"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_pswd"), XSDL_STY_BCK_COL, &mns_input_bck_col);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_pswd"), XSDL_STY_BOR, &one);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_pswd"), XSDL_STY_BOR_COL, &mns_input_bor_col);
	pswd_input = XSDL_Get(window->ui_root, "mns_pswd")->element;
	memcpy(&pswd_input->col, &mns_text_col, sizeof(XSDL_Color));

	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_form"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_form"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_form"), XSDL_STY_BCK_COL, &mns_form_bck_col);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_form"), XSDL_STY_COR_RAD, &mns_form_corners);

	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_title"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_title"), XSDL_STY_BCK_COL, &mns_title_bck_col);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_title"), XSDL_STY_COR_RAD, &mns_title_cor);

	XSDL_BindEvent(XSDL_Get(window->ui_root, "mns_login"), XSDL_EVT_MOUSEDOWN, &try_login);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_login"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(window->ui_root, "mns_login"), XSDL_STY_BCK_COL, &mns_login_bck_col);

	XSDL_BuildPipe(window->ui_ctx->pipe, window->ui_root);

	return(0);
}
