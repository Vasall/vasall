#include <stdio.h>
#include <stdlib.h>
#include "gui.h"

void try_login()
{
	printf("Trying to connect to server...\n");

	ENUD_ModFlag(ENUD_Get(g_root, "mns"), ENUD_FLG_ACT, &zero);
	ENUD_ModFlag(ENUD_Get(g_root, "gms"), ENUD_FLG_ACT, &one);

	g_procevt = &game_procevt;
	g_update = &game_update;

	return;
}

/*
 * Initialize the GUI and place all elements.
 */
void init_gui()
{
	/* Create the menu-sceen */
	ENUD_CreateWrapper(g_root, "mns", 0, 0, -100, -100);
	ENUD_CreateWrapper(ENUD_Get(g_root, "mns"), "mns_form",
			-1, -1, 400, 380);

	ENUD_CreateWrapper(ENUD_Get(g_root, "mns_form"), "mns_title",
			0, 0, 400, 80);
	ENUD_Rect body0 = {50, 14, 300, 52};
	ENUD_CreateText(ENUD_Get(g_root, "mns_title"), "label0", &body0,
			"VASALL", &ENUD_WHITE, 2, 0);

	ENUD_Rect body1 = {40, 106, 320, 24};
	ENUD_CreateText(ENUD_Get(g_root, "mns_form"), "label1", &body1,"Email:", 
			&ENUD_WHITE, 1, ENUD_TEXT_LEFT);
	ENUD_CreateInput(ENUD_Get(g_root, "mns_form"), "mns_user", 40, 130, 320, 40, "");

	ENUD_Rect body2 = {40, 186, 320, 24};
	ENUD_CreateText(ENUD_Get(g_root, "mns_form"), "label2", &body2, "Password:", 
			&ENUD_WHITE, 1, ENUD_TEXT_LEFT);
	ENUD_CreateInput(ENUD_Get(g_root, "mns_form"), "mns_pswd", 40, 210, 320, 40, "");

	ENUD_CreateButton(ENUD_Get(g_root, "mns_form"), "mns_login", 40, 280, 320, 40, "Login");

	ENUD_Color mns_form_bck_col = {0x23, 0x23, 0x23, 0xff};
	short mns_form_corners[] = {5, 5, 5, 5};
	ENUD_ModStyle(ENUD_Get(g_root, "mns_form"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_form"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_form"), ENUD_STY_BCK_COL, &mns_form_bck_col);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_form"), ENUD_STY_COR_RAD, &mns_form_corners);

	ENUD_Color mns_title_bck_col = {0xd3, 0x34, 0x5a, 0xff};
	short mns_title_cor[] = {5, 5, 0, 0};
	ENUD_ModStyle(ENUD_Get(g_root, "mns_title"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_title"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_title"), ENUD_STY_BCK_COL, &mns_title_bck_col);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_title"), ENUD_STY_COR_RAD, &mns_title_cor);

	ENUD_BindEvent(ENUD_Get(g_root, "mns_login"), ENUD_EVT_MOUSEDOWN, &try_login);

	/* Create the game-sceen */
	ENUD_CreateWrapper(g_root, "gms", 0, 0, -100, -100);
	ENUD_ModStyle(ENUD_Get(g_root, "gms"), ENUD_STY_VIS, &one);

	ENUD_CreateWrapper(ENUD_Get(g_root, "gms"), "gms_stats", -1, 5, 790, 35);
	ENUD_Color gms_stats_bck_col = {0x23, 0x25, 0x30, 0xff};
	short gms_stats_cor[] = {6, 6, 6, 6};
	ENUD_ModStyle(ENUD_Get(g_root, "gms_stats"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "gms_stats"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "gms_stats"), ENUD_STY_BCK_COL, &gms_stats_bck_col);
	ENUD_ModStyle(ENUD_Get(g_root, "gms_stats"), ENUD_STY_COR_RAD, &gms_stats_cor);

	ENUD_ModFlag(ENUD_Get(g_root, "gms"), ENUD_FLG_ACT, &zero);
}
