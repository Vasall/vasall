#include "setup.h"
#include "ui_stdnode.h"

#include <stdlib.h>

int load_resources(void)
{
	char *vars1[3] = {"vtxPos\0", "vtxTex\0", "vtxNrm\0"};
	char *vars2[5] = {"vtxPos\0", "vtxTex\0", "vtxNrm\0", "vtxJnt\0", "vtxWgt\0"};
	char *pths[6] = {"res/textures/px.png", "res/textures/nx.png", "res/textures/py.png", "res/textures/ny.png", "res/textures/pz.png", "res/textures/nz.png"};

	printf("Start loading resources\n");

	/* fonts */
	if(txt_load_ttf("res/fonts/mecha.ttf", 24) < 0)
		return -1;

	if(txt_load_ttf("res/fonts/roboto_reg.ttf", 16) < 0)
		return -1;

	if(txt_load_ttf("res/fonts/aller.ttf", 16) < 0)
		return -1;

	if(txt_load_ttf("res/fonts/roboto_bold.ttf", 28) < 0)
		return -1;


	/* shaders */
	if(shd_set("mdl", "res/shaders/model.vert", "res/shaders/model.frag", 3, vars1, MDL_TYPE_DEFAULT) < 0)
		return -1;

	if(shd_set("ani", "res/shaders/animated.vert", "res/shaders/animated.frag", 5, vars2, MDL_TYPE_DEFAULT) < 0)
		return -1;

	if(shd_set("skybox", "res/shaders/skybox.vert", "res/shaders/skybox.frag", 3, vars1, MDL_TYPE_SKYBOX) < 0)
		return -1;


	/* textures */
	if(tex_set("ba0", "res/textures/red.png") < 0)
		return -1;

	if(tex_set("ba1", "res/textures/blue.png") < 0)
		return -1;

	if(tex_set("ba2", "res/textures/green.png") < 0)
		return -1;

	if(tex_set("flr", "res/textures/floor.png") < 0)
		return -1;

	if(tex_set("pal", "res/textures/palette.png") < 0)
		return -1;

	if(skybox_set("skybox", pths) < 0)
		return -1;

	/* models */
	if(mdl_load("wld", "res/models/plane.amo", tex_get("flr"), shd_get("mdl"), MDL_TYPE_DEFAULT) < 0)
		return -1;

	if(mdl_load("plr", "res/models/player.amo", tex_get("ba0"), shd_get("ani"), MDL_TYPE_DEFAULT) < 0)
		return -1;

	if(mdl_load("slp", "res/models/slope.amo", tex_get("ba1"), shd_get("mdl"), MDL_TYPE_DEFAULT) < 0)
			return -1;

	if(mdl_load("gun", "res/models/pistol.obj", tex_get("ba1"), shd_get("mdl"), MDL_TYPE_DEFAULT) < 0)
			return -1;

	if(mdl_load("sph", "res/models/sphere.obj", tex_get("ba2"), shd_get("mdl"), MDL_TYPE_DEFAULT) < 0)
			return -1;


	if(mdl_load("tst", "res/models/test.amo", tex_get("ba0"), shd_get("mdl"), MDL_TYPE_DEFAULT) < 0)
			return -1;


	if(mdl_load("cube", "res/models/cube.obj", tex_get("skybox"), shd_get("skybox"), MDL_TYPE_SKYBOX) < 0)
		abort();

	/* handhelds */
	if(hnd_load("res/models/pistol.hnd", tex_get("pal"), shd_get("mdl")) < 0)
		return -1;

	printf("Finished loading resources\n");
	return 0;
}

/* If login was successfull */
extern void test1(char *buf, int len)
{
	ui_node *node;
	char zero = 0;
	float asp;

	uint32_t id = 10000;

	vec3_t pos0 = {0.0, 0.0, 0.0};
	vec3_t pos1 = {0.0, -12.0, 0.0};
	vec3_t pos2 = {0.0, 15.0, 0.0};

	if(buf || len) {/* Prevent warning for not using parameters */}

	/* Place a slope */
	obj_set(id, OBJ_M_STATIC, pos0, mdl_get("wld"), NULL, 0, 0);
	obj_set(id + 101, OBJ_M_STATIC, pos1, mdl_get("slp"), NULL, 0, 0);
	obj_set(id + 102, OBJ_M_STATIC, pos2, mdl_get("tst"), NULL, 0, 0);

	/* Set the skybox */
	wld_set_skybox(mdl_get("cube"));

	/* Setup camera */
	cam_trg_obj(g_core.obj);
	g_cam.dist = 3.0;

	/* Update projection-matrix */
	asp = (float)g_win.win_w / (float)g_win.win_h;
	cam_proj_mat(45.0, asp, 0.1, 1000.0);

	/* Switch from menuscreen to gamescreen */
	node = ui_get(g_win.root, "mns");
	ui_set_flag(node, FLG_ACT, &zero);

	SDL_SetRelativeMouseMode(SDL_TRUE);

	/* Start the game-loop */
	game_start();
}

/* If login failed */
extern void test2(char *buf, int len)
{
	if(buf || len) {/* Prevent warning for not using parameters */}
	printf("Login failed! Check username and password.\n");
}

/* Login into the server and insert peer into cluster */
extern void login(ui_node *n, event_t *e)
{
	char uname[17];
	char pswd[65];
	ui_node *node;
	ui_input *inp;

	node = ui_get(g_win.root, "mns_user");
	inp = node->element;
	strcpy(uname, inp->buffer);

	if(strlen(uname) < 5) {
		printf("Please insert an username!\n");
		return;
	}

	node = ui_get(g_win.root, "mns_pswd");
	inp = node->element;
	strcpy(pswd, inp->buffer);

	if(strlen(pswd) == 0) {
		printf("Please insert your password!\n");
		return;
	}

	if(n || e){/* Prevent warnings for not using parameters */}

	printf("Send to server\n");
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
	ui_node *root = g_win.root;
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


	ele = ui_new_text("USERNAME\0", sdl_color(255, 255, 255, 255), 2, TXT_LEFT);
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
	ui_style(tmp,  UI_STY_BCKCOL, sdl_color_s(0x25, 0x25, 0x25, 0xff));
	val = 1;
	ui_style(tmp,  UI_STY_BOR,    &val);
	ui_style(tmp,  UI_STY_BORCOL, sdl_color_s(0x10, 0x10, 0x10, 0xff));
	ui_style(tmp,  UI_STY_CRNRAD, short4(2, 2, 2, 2));
	memcpy(&((ui_input *)ele)->cur_col, sdl_color_s(255, 255, 255, 255), sizeof(color_t));
	ui_enable_tex(tmp);


	ele = ui_new_text("PASSWORD\0", sdl_color(255, 255, 255, 255), 2, TXT_LEFT);
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
	ui_constr(tmp, UI_CST_POS, UI_CST_VERT, 0, UI_CST_TOP, 210, UI_CST_PX, 0);
	ui_style(tmp, UI_STY_BCK, &one);
	ui_style(tmp, UI_STY_BCKCOL, sdl_color_s(0x25, 0x25, 0x25, 0xff));
	val = 1;
	ui_style(tmp, UI_STY_BOR, &val);
	ui_style(tmp, UI_STY_BORCOL, sdl_color_s(0x10, 0x10, 0x10, 0xff));
	ui_style(tmp, UI_STY_CRNRAD, short4(2, 2, 2, 2));
	memcpy(&((ui_input *)ele)->cur_col, sdl_color_s(255, 255, 255, 255), sizeof(color_t));
	ui_enable_tex(tmp);


	ele = ui_new_button(&login);
	tmp = ui_add(UI_BUTTON, ui_get(root, "mns_form"), ele, "mns_login_btn");
	ui_constr(tmp, UI_CST_SIZE, UI_CST_HORI, 0, 1,           200, UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_SIZE, UI_CST_VERT, 0, 1,           42,  UI_CST_PX, 0);
	ui_constr(tmp, UI_CST_POS,  UI_CST_HORI, 0, UI_CST_AUTO, 0,   0,         0);
	ui_constr(tmp, UI_CST_POS,  UI_CST_VERT, 0, UI_CST_TOP,  280, UI_CST_PX, 0);
	ui_style(tmp, UI_STY_BCK, &one);
	ui_style(tmp, UI_STY_BCKCOL, sdl_color_s(0x0c, 0xaa, 0xdc, 0xff));
	ui_style(tmp, UI_STY_CRNRAD, short4(2, 2, 2, 2));

	ele = ui_new_text("Login\0", sdl_color(255, 255, 255, 255), 2, 0);
	tmp = ui_add(UI_TEXT, ui_get(root, "mns_login_btn"), ele, "mns_login_lbl");

	/* Chain elements so tab can be used to iterate throught elements */
	ui_chain(3, "mns_user", "mns_pswd", "mns_login_btn");

	win_build_pipe();

	/* Auto-focus the username-input */
	win_focus_node(ui_get(root, "mns_user"));
	return 0;
}

