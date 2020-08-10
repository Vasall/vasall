#include "setup.h"
#include "stdnode.h"

#include <stdlib.h>

int load_resources(void)
{
	if(txt_font_ttf("res/fonts/mecha.ttf", 24) < 0)
		return -1;

	if(txt_font_ttf("res/fonts/roboto_reg.ttf", 16) < 0)
		return -1;

	if(txt_font_ttf("res/fonts/aller.ttf", 16) < 0)
		return -1;

	if(txt_font_ttf("res/fonts/roboto_bold.ttf", 28) < 0)
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
	float asp;

	vec3_t dir = {-1.0, 1.0, -1.0};

	if(buf || len) {/* Prevent warning for not using parameters */}

	/* Setup camera */
	cam_trg_obj(core.obj);
	camera.dist = 10.0;
	cam_set_dir(dir);

	/* Update projection-matrix */
	asp = (float)window.win_w / (float)window.win_h;
	cam_proj_mat(45.0, asp, 0.1, 1000.0);

	/* Switch from menuscreen to gamescreen */
	node = ui_get(window.root, "mns");
	ui_set_flag(node, FLG_ACT, &zero);

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

	node = ui_get(window.root, "mns_user");
	inp = node->element;
	strcpy(uname, inp->buffer);

	node = ui_get(window.root, "mns_pswd");
	inp = node->element;
	strcpy(pswd, inp->buffer);

	if(n || e){/* Prevent warnings for not using parameters */}

	net_insert(uname, pswd, &test1, &test2);
}


static short *short4(short a, short b, short c, short d)
{
	static short arr[4] = {0, 0, 0, 0};

	arr[0] = a;
	arr[1] = b;
	arr[2] = c;
	arr[3] = d;
		
	return arr;
}

int load_ui(void)
{
	ui_node *root = window.root;
	ui_node *tmp;
	char one = 1;
	short val;
	void *ele;

	tmp = ui_add(UI_WRAPPER, ui_get(root, "root"), NULL, "mns");
	ui_enable_tex(tmp);

	tmp = ui_add(UI_WRAPPER, ui_get(root, "mns"), NULL, "mns_form");
	ui_constr(tmp, UI_CST_SIZE, UI_CST_HORI, 0, 1, 400, UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_SIZE, UI_CST_VERT, 0, 1, 380, UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_POS, UI_CST_HORI, 0, UI_CST_AUTO, 0, 0, 0);
	ui_constr(tmp, UI_CST_POS, UI_CST_VERT, 0, UI_CST_AUTO, 0, 0, 0);
	ui_style(tmp, UI_STY_VIS, &one);
	ui_style(tmp, UI_STY_BCK, &one);
	ui_style(tmp, UI_STY_BCKCOL, sdl_color_s(0x24, 0x27, 0x29, 0xff));
	val = 1;
	ui_style(tmp, UI_STY_BOR, &val);
	ui_style(tmp, UI_STY_BORCOL, sdl_color_s(0x09, 0x09, 0x09, 0xff));
	ui_style(tmp, UI_STY_CRNRAD, short4(4, 4, 4, 4));

	ele = ui_new_text("VASALL\0", sdl_color(255, 255, 255, 255), 3, 0);
	tmp = ui_add(UI_TEXT, ui_get(root, "mns_form"), ele, "mns_title");
	ui_constr(tmp, UI_CST_SIZE, UI_CST_VERT, 0, 1, 89, UI_CST_PX, 0);


	ele = ui_new_text("USERNAME\0", sdl_color(255, 255, 255, 255), 2, TEXT_LEFT);
	tmp = ui_add(UI_TEXT, ui_get(root, "mns_form"), ele, "mns_user_lbl");
	ui_constr(tmp, UI_CST_SIZE, UI_CST_HORI, 0, 1,           350, UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_SIZE, UI_CST_VERT, 0, 1,           22,  UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_POS,  UI_CST_HORI, 0, UI_CST_AUTO, 0,   0,         0);
	ui_constr(tmp, UI_CST_POS,  UI_CST_VERT, 0, UI_CST_TOP,  100, UI_CST_PX, 0);

	ele = ui_new_input(sdl_color(0xf9, 0xf9, 0xf9, 0xff), short4(0, 5, 0, 5));
	tmp = ui_add(UI_INPUT, ui_get(root, "mns_form"), ele, "mns_user");
	ui_constr(tmp, UI_CST_SIZE, UI_CST_HORI, 0, 1,           350, UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_SIZE, UI_CST_VERT, 0, 1,           42,  UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_POS,  UI_CST_HORI, 0, UI_CST_AUTO, 0,   0,         0);
	ui_constr(tmp, UI_CST_POS,  UI_CST_VERT, 0, UI_CST_TOP,  125, UI_CST_PX, 0);
	ui_style(tmp,  UI_STY_BCK,    &one);
	ui_style(tmp,  UI_STY_BCKCOL, sdl_color_s(0x21, 0x21, 0x21, 0xff));
	val = 1;
	ui_style(tmp,  UI_STY_BOR,    &val);
	ui_style(tmp,  UI_STY_BORCOL, sdl_color_s(0x10, 0x10, 0x10, 0xff));
	ui_style(tmp,  UI_STY_CRNRAD, short4(2, 2, 2, 2));
	memcpy(&((ui_input *)ele)->cur_col, sdl_color_s(255, 255, 255, 255), sizeof(color_t));
	ui_enable_tex(tmp);


	ele = ui_new_text("PASSWORD\0", sdl_color(255, 255, 255, 255), 2, TEXT_LEFT);
	tmp = ui_add(UI_TEXT, ui_get(root, "mns_form"), ele, "mns_user_lbl");
	ui_constr(tmp, UI_CST_SIZE, UI_CST_HORI, 0, 1,           350, UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_SIZE, UI_CST_VERT, 0, 1,           22,  UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_POS,  UI_CST_HORI, 0, UI_CST_AUTO, 0,   0,         0);
	ui_constr(tmp, UI_CST_POS,  UI_CST_VERT, 0, UI_CST_TOP,  185, UI_CST_PX, 0);

	ele = ui_new_input(sdl_color(0xf9, 0xf9, 0xf9, 255), short4(0, 5, 0, 5));
	tmp = ui_add(UI_INPUT, ui_get(root, "mns_form"), ele, "mns_pswd");
	ui_constr(tmp, UI_CST_SIZE, UI_CST_HORI, 0, 1, 350, UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_SIZE, UI_CST_VERT, 0, 1, 42, UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_POS, UI_CST_HORI, 0, UI_CST_AUTO, 0, 0, 0);
	ui_style(tmp, UI_STY_BCK, &one);
	ui_style(tmp, UI_STY_BCKCOL, sdl_color_s(0x21, 0x21, 0x21, 0xff));
	ui_constr(tmp, UI_CST_POS, UI_CST_VERT, 0, UI_CST_TOP, 210, UI_CST_PX, 0);
	val = 1;
	ui_style(tmp, UI_STY_BOR, &val);
	ui_style(tmp, UI_STY_BORCOL, sdl_color_s(0x10, 0x10, 0x10, 0xff));
	ui_style(tmp, UI_STY_CRNRAD, short4(2, 2, 2, 2));
	memcpy(&((ui_input *)ele)->cur_col, sdl_color_s(255, 255, 255, 255), sizeof(color_t));
	ui_enable_tex(tmp);


	tmp = ui_add(UI_BUTTON, ui_get(root, "mns_form"), NULL, "mns_login_btn");
	ui_constr(tmp, UI_CST_SIZE, UI_CST_HORI, 0, 1,           200, UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_SIZE, UI_CST_VERT, 0, 1,           42,  UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_POS,  UI_CST_HORI, 0, UI_CST_AUTO, 0,   0,         0);
	ui_constr(tmp, UI_CST_POS,  UI_CST_VERT, 0, UI_CST_TOP,  280, UI_CST_PX, 0);
	ui_style(tmp, UI_STY_BCK, &one);
	ui_style(tmp, UI_STY_BCKCOL, sdl_color_s(0x0c, 0xaa, 0xdc, 0xff));
	ui_style(tmp, UI_STY_CRNRAD, short4(2, 2, 2, 2));
	ui_event(tmp, EVT_MOUSEDOWN, &login);

	ele = ui_new_text("Login\0", sdl_color(255, 255, 255, 255), 2, 0);
	tmp = ui_add(UI_TEXT, ui_get(root, "mns_login_btn"), ele, "mns_login_lbl");

	win_build_pipe();

	win_dump_pipe();

#if 0
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
#endif
	return 0;
}

