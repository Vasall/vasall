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

	if(shd_set("col", "res/shaders/color.vert", "res/shaders/color.frag") < 0)
		return -1;

	if(shd_set("uv", "res/shaders/uv.vert", "res/shaders/uv.frag") < 0)
		return -1;

	if(tex_load_png("plr", "res/textures/base.png") < 0)
		return -1;

	if(mdl_load("plr", "res/models/base.obj", tex_get("plr"), shd_get("uv")) < 0)
		return -1;

	return 0;
}

/* If login was successfull */
static void test1(char *buf, int len)
{
	ui_node *node;
	char zero = 0;

	vec3_t dir = {-1.0, 1.0, -1.0};

	if(buf || len) {/* Prevent warning for not using parameters */}

	/* Setup camera */
	cam_trg_obj(core.obj);
	camera.dist = 10.0;
	cam_set_dir(dir);

	/* Switch from menuscreen to gamescreen */
	node = ui_get("mns");
	ui_mod_flag(node, FLG_ACT, &zero);

	/* Start the game-loop */
	game_start();
}

/* If login failed */
static void test2(char *buf, int len)
{
	if(buf || len) {/* Prevent warning for not using parameters */}
	printf("Failed\n");
}

/* Login into the server and insert peer into cluster */
static void login(ui_node *n, SDL_Event *e)
{
	char uname[17];
	char pswd[65];
	struct ui_node *node;
	struct ui_input *inp;

	node = ui_get("mns_user");
	inp = node->element;
	strcpy(uname, inp->buffer);

	node = ui_get("mns_pswd");
	inp = node->element;
	strcpy(pswd, inp->buffer);

	if(n || e){/* Prevent warnings for not using parameters */}

	net_insert(uname, pswd, &test1, &test2);
}


int load_ui(void)
{
	int one = 1;
	rect_t body0 = {0, 0, 400, 80};
	rect_t body1 = {40, 96, 320, 24};
	rect_t body2 = {40, 176, 320, 24};
	struct ui_node *tmp;
	void *ele;

	/* 
	 * FIXME:
	 * If at this very position, there are not bytes, the UI will not be 
	 * rendered correctly.
	 */

	color_t mns_input_bck_col = {0x37, 0x37, 0x37, 0xFF};
	color_t mns_input_bor_col = {0x28, 0x28, 0x28, 0xFF};
	color_t mns_form_bck_col = {0x3D, 0x3B, 0x3C, 0xfb};
	short mns_form_corners[4] = {8, 8, 8, 8};
	color_t mns_title_bck_col = {0xd3, 0x34, 0x5a, 0xff};
	color_t mns_login_bck_col = {0xff, 0x00, 0x00, 0xff};
	short mns_title_cor[] = {5, 5, 0, 0};

	/* Create the menu-sceen */
	ui_add(UI_WRAPPER, ui_get("root"), "mns", NULL, sdl_rect(0, 0, 800, 600));
	ui_add(UI_WRAPPER, ui_get("mns"), "mns_form", NULL, sdl_rect(200, 80, 400, 380));

	ui_add(UI_WRAPPER, ui_get("mns_form"), "mns_title", NULL, sdl_rect(0, 0, 400, 80));

	ele = ui_new_text("VASALL\0", sdl_color(0xff, 0xff, 0xff, 0xff), 3, 0);
	ui_add(UI_TEXT, ui_get("mns_title"), "label0", ele, sdl_rect(0, 0, 400, 80));

	ele = ui_new_text("Username\0", sdl_color(0xff, 0xff, 0xff, 0xff, 2, TEXT_LEFT));
	ui_add(UI_TEXT, ui_get("mns_form"), "label1", ele, sdl_rect(40, 96, 320, 24));


	ui_add(UI_INPUT, ui_get("mns_form"), "mns_user", 40, 120, 320, 40, "");


	ui_add_text(ui_get("mns_form"), "label2", &body2, "Password:", &WHITE, 2, TEXT_LEFT);
	ui_add_input(ui_get("mns_form"), "mns_pswd", 40, 200, 320, 40, "");
	/* ((ui_input *)ui_get("mns_pswd")->element)->hide = 1; */

	ui_add_button(ui_get("mns_form"), "mns_login", 40, 270, 320, 40, "Login");

	tmp = ui_get("mns_user");
	ui_mod_style(tmp, STY_VIS, &one);
	ui_mod_style(tmp, STY_BCK, &one);
	ui_mod_style(tmp, STY_BCK_COL, &mns_input_bck_col);
	ui_mod_style(tmp, STY_BOR, &one);
	ui_mod_style(tmp, STY_BOR_COL, &mns_input_bor_col);

	tmp = ui_get("mns_pswd");
	ui_mod_style(tmp, STY_VIS, &one);
	ui_mod_style(tmp, STY_BCK, &one);
	ui_mod_style(tmp, STY_BCK_COL, &mns_input_bck_col);
	ui_mod_style(tmp, STY_BOR, &one);
	ui_mod_style(tmp, STY_BOR_COL, &mns_input_bor_col);

	tmp = ui_get("mns_form");
	ui_mod_style(tmp, STY_VIS, &one);
	ui_mod_style(tmp, STY_BCK, &one);
	ui_mod_style(tmp, STY_BCK_COL, &mns_form_bck_col);
	ui_mod_style(tmp, STY_COR_RAD, &mns_form_corners);

	tmp = ui_get("mns_title");
	ui_mod_style(tmp, STY_VIS, &one);
	ui_mod_style(tmp, STY_BCK_COL, &mns_title_bck_col);
	ui_mod_style(tmp, STY_COR_RAD, &mns_title_cor);

	tmp = ui_get("mns_login");
	ui_bind_event(tmp, EVT_MOUSEDOWN, &login);
	ui_mod_style(tmp, STY_BCK, &one);
	ui_mod_style(tmp, STY_BCK_COL, &mns_login_bck_col);

	win_build_pipe();
	return 0;
}

