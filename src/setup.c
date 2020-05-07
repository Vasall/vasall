#include "setup.h"

#include <stdlib.h>

int load_resources(void)
{
	if(txt_font_ttf("res/fonts/mecha.ttf", 24) < 0)
		return -1;

	if(txt_font_ttf("res/fonts/aller.ttf", 16) < 0)
		return -1;

	if(txt_font_ttf("res/fonts/aller.ttf", 16) < 0)
		return -1;

	if(txt_font_ttf("res/fonts/editundo.ttf", 48) < 0)
		return -1;

	if(shd_set("col", "res/shader/color.vert", "res/shader/color.frag") < 0)
		return -1;

	if(shd_set("uv", "res/shader/uv.vert", "res/shader/uv.frag") < 0)
		return -1;

	if(tex_load_png("plr", "res/texture/finn2.png") < 0)
		return -1;

	if(mdl_load("plr", "res/model/finn.obj", tex_get("plr"), shd_get("uv")) < 0)
		return -1;

	return 0;
}

static void try_login(ui_node *n, SDL_Event *e)
{
	ui_node *node;
	char zero = 0;

	if(n || e){/* Prevent warnings for not using parameters */}

	/* Update core functions */
	core.proc_evt = &game_proc_evt;
	core.update = &game_update;
	core.render = &game_render;

	/* Switch from menuscreen to gamescreen */
	node = ui_get("mns");
	ui_mod_flag(node, FLG_ACT, &zero);
}

int load_ui(void)
{
	char one = 1;
	SDL_Rect body0 = {0, 0, 400, 80};
	SDL_Rect body1 = {40, 96, 320, 24};
	SDL_Rect body2 = {40, 176, 320, 24};
	SDL_Color mns_text_col = { 0xF9, 0xF9, 0xF9, 0xFF };
	SDL_Color mns_input_bck_col = {0x37, 0x37, 0x37, 0xFF};
	SDL_Color mns_input_bor_col = {0x28, 0x28, 0x28, 0xFF};
	SDL_Color mns_form_bck_col = {0x3D, 0x3B, 0x3C, 0xfb};
	short mns_form_corners[] = {8, 8, 8, 8};
	SDL_Color mns_title_bck_col = {0xd3, 0x34, 0x5a, 0xff};
	SDL_Color mns_login_bck_col = {0xff, 0x00, 0x00, 0xff};
	short mns_title_cor[] = {5, 5, 0, 0};

	/* Create the menu-sceen */
	ui_add_wrapper(ui_get("root"), "mns", 0, 0, 800, 600);
	ui_add_wrapper(ui_get("mns"), "mns_form", 200, 80, 400, 380);

	ui_add_wrapper(ui_get("mns_form"), "mns_title", 0, 0, 400, 80);
	ui_add_text(ui_get("mns_title"), "label0", &body0, "VASALL", &WHITE, 3, 0);

	ui_add_text(ui_get("mns_form"), "label1", &body1, "Email:", &WHITE, 2, TEXT_LEFT);
	ui_add_input(ui_get("mns_form"), "mns_user", 40, 120, 320, 40, "");

	ui_add_text(ui_get("mns_form"), "label2", &body2, "Password:", &WHITE, 2, TEXT_LEFT);
	ui_add_input(ui_get("mns_form"), "mns_pswd", 40, 200, 320, 40, "");
	/* ((ui_input *)ui_get("mns_pswd")->element)->hide = 1; */

	ui_add_button(ui_get("mns_form"), "mns_login", 40, 270, 320, 40, "Login");

	ui_mod_style(ui_get("mns_user"), STY_VIS, &one);
	ui_mod_style(ui_get("mns_user"), STY_BCK, &one);
	ui_mod_style(ui_get("mns_user"), STY_BCK_COL, &mns_input_bck_col);
	ui_mod_style(ui_get("mns_user"), STY_BOR, &one);
	ui_mod_style(ui_get("mns_user"), STY_BOR_COL, &mns_input_bor_col);

	ui_mod_style(ui_get("mns_pswd"), STY_VIS, &one);
	ui_mod_style(ui_get("mns_pswd"), STY_BCK, &one);
	ui_mod_style(ui_get("mns_pswd"), STY_BCK_COL, &mns_input_bck_col);
	ui_mod_style(ui_get("mns_pswd"), STY_BOR, &one);
	ui_mod_style(ui_get("mns_pswd"), STY_BOR_COL, &mns_input_bor_col);

	ui_mod_style(ui_get("mns_form"), STY_VIS, &one);
	ui_mod_style(ui_get("mns_form"), STY_BCK, &one);
	ui_mod_style(ui_get("mns_form"), STY_BCK_COL, &mns_form_bck_col);
	ui_mod_style(ui_get("mns_form"), STY_COR_RAD, &mns_form_corners);

	ui_mod_style(ui_get("mns_title"), STY_VIS, &one);
	ui_mod_style(ui_get("mns_title"), STY_BCK_COL, &mns_title_bck_col);
	ui_mod_style(ui_get("mns_title"), STY_COR_RAD, &mns_title_cor);

	ui_bind_event(ui_get("mns_login"), EVT_MOUSEDOWN, &try_login);
	ui_mod_style(ui_get("mns_login"), STY_BCK, &one);
	ui_mod_style(ui_get("mns_login"), STY_BCK_COL, &mns_login_bck_col);

	win_build_pipe();
	win_dump_pipe();
	return 0;
}

