#include <stdio.h>
#include <stdlib.h>

#include "gui.h"


void try_login()
{
	printf("Trying to connect to server...\n");

	ENUD_ModFlag(ENUD_Get(g_root, "mns"), ENUD_FLG_ACT, &zero);
	ENUD_ModFlag(ENUD_Get(g_root, "gms"), ENUD_FLG_ACT, &one);

	/*ENUD_Color new_col = {0x1d, 0x43, 0x73, 0xFF };*/
	/*ENUD_Color new_col = { 0xE0, 0xAB, 0x7B, 0xFF };*/
	ENUD_Color new_col = { 0x23, 0x23, 0x23, 0xFF };
	memcpy(&g_win_clr, &new_col, sizeof(ENUD_Color));

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
			"VASALL", &ENUD_WHITE, 3, 0);

	ENUD_Rect body1 = {40, 96, 320, 24};
	ENUD_CreateText(ENUD_Get(g_root, "mns_form"), "label1", &body1,"Email:", 
			&ENUD_WHITE, 2, ENUD_TEXT_LEFT);
	ENUD_CreateInput(ENUD_Get(g_root, "mns_form"), "mns_user", 40, 120, 320, 40, "");

	ENUD_Rect body2 = {40, 176, 320, 24};
	ENUD_CreateText(ENUD_Get(g_root, "mns_form"), "label2", &body2, "Password:", 
			&ENUD_WHITE, 2, ENUD_TEXT_LEFT);
	ENUD_CreateInput(ENUD_Get(g_root, "mns_form"), "mns_pswd", 40, 200, 320, 40, "");

	ENUD_CreateButton(ENUD_Get(g_root, "mns_form"), "mns_login", 40, 270, 320, 40, "Login");

	ENUD_Color mns_text_col = { 0xF9, 0xF9, 0xF9, 0xFF };
	ENUD_Color mns_input_bck_col = {0x37, 0x37, 0x37, 0xFF};
	ENUD_Color mns_input_bor_col = {0x28, 0x28, 0x28, 0xFF};
	short mns_input_corners[] = {4, 4, 4, 4};
	ENUD_ModStyle(ENUD_Get(g_root, "mns_user"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_user"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_user"), ENUD_STY_BCK_COL, &mns_input_bck_col);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_user"), ENUD_STY_BOR, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_user"), ENUD_STY_BOR_COL, &mns_input_bor_col);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_user"), ENUD_STY_COR_RAD, &mns_input_corners);
	ENUD_Input *user_input = ENUD_Get(g_root, "mns_user")->element;
	memcpy(&user_input->col, &mns_text_col, sizeof(ENUD_Color));

	ENUD_ModStyle(ENUD_Get(g_root, "mns_pswd"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_pswd"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_pswd"), ENUD_STY_BCK_COL, &mns_input_bck_col);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_pswd"), ENUD_STY_BOR, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_pswd"), ENUD_STY_BOR_COL, &mns_input_bor_col);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_pswd"), ENUD_STY_COR_RAD, &mns_input_corners);
	ENUD_Input *pswd_input = ENUD_Get(g_root, "mns_pswd")->element;
	memcpy(&pswd_input->col, &mns_text_col, sizeof(ENUD_Color));


	ENUD_Color mns_form_bck_col = {0x3D, 0x3B, 0x3C, 0xff};
	short mns_form_corners[] = {5, 5, 5, 5};
	ENUD_ModStyle(ENUD_Get(g_root, "mns_form"), ENUD_STY_VIS, &one);
	/*ENUD_ModStyle(ENUD_Get(g_root, "mns_form"), ENUD_STY_BCK, &one);*/
	ENUD_ModStyle(ENUD_Get(g_root, "mns_form"), ENUD_STY_BCK_COL, &mns_form_bck_col);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_form"), ENUD_STY_COR_RAD, &mns_form_corners);

	ENUD_Color mns_title_bck_col = {0xd3, 0x34, 0x5a, 0xff};
	short mns_title_cor[] = {5, 5, 0, 0};
	ENUD_ModStyle(ENUD_Get(g_root, "mns_title"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_title"), ENUD_STY_BCK_COL, &mns_title_bck_col);
	ENUD_ModStyle(ENUD_Get(g_root, "mns_title"), ENUD_STY_COR_RAD, &mns_title_cor);

	ENUD_BindEvent(ENUD_Get(g_root, "mns_login"), ENUD_EVT_MOUSEDOWN, &try_login);

	/* Create the game-sceen */
	ENUD_CreateWrapper(g_root, "gms", 0, 0, -100, -100);
	ENUD_ModStyle(ENUD_Get(g_root, "gms"), ENUD_STY_VIS, &one);

	ENUD_CreateWrapper(ENUD_Get(g_root, "gms"), "gms_stats", -1, 5, 790, 35);
	ENUD_Color gms_stats_bck_col = { 0x2b, 0x2b, 0x36, 0xf8};
	short gms_stats_cor[] = {3, 3, 3, 3};
	ENUD_ModStyle(ENUD_Get(g_root, "gms_stats"), ENUD_STY_VIS, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "gms_stats"), ENUD_STY_BCK, &one);
	ENUD_ModStyle(ENUD_Get(g_root, "gms_stats"), ENUD_STY_BCK_COL, &gms_stats_bck_col);
	ENUD_ModStyle(ENUD_Get(g_root, "gms_stats"), ENUD_STY_COR_RAD, &gms_stats_cor);

	ENUD_ModFlag(ENUD_Get(g_root, "gms"), ENUD_FLG_ACT, &zero);
}
