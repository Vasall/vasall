/* Using SDL and standard IO */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
/* Include custom headers */
#include "../XSDL/xsdl.h"
#include "vector.h"

/* Window setting */
int win_w = 800;
int win_h = 600;
uint32_t win_flgs = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

/* === Global variables === */
uint8_t one = 1;
uint8_t zero = 0;

uint32_t running = 0;
uint32_t fullscreen = 0;

XSDL_Window *window;
XSDL_GLContext *gl_ctx;
XSDL_UIContext *ui_ctx;

/* === Prototypes === */
int init();
XSDL_Window *init_window();
XSDL_GLContext *init_glcontext(XSDL_Window *win);
void init_gui(XSDL_UIContext *ctx);

int main(int argc, char** argv) 
{
	printf("\nStarting vasall-client...\n");

	char *pth = "/home/juke/code/c/build/res/shader/demo_vss.glsl";
	char *buf;
	int len;
	int a = ReadShaderFile(pth, &buf, &len);
	printf("a: %d\n", a);
	if(a < 0) 
		return(0);

	return 1;

	if(init() < 0)
		goto exit;

	init_gui(ui_ctx);

	XSDL_FillRect(0, 0, 0, 0);

	running = 1;
	
	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE:
						running = 0;
						break;
					case 'f':
						fullscreen = !fullscreen;
						if (fullscreen) {
							SDL_SetWindowFullscreen(window, win_flgs | SDL_WINDOW_FULLSCREEN_DESKTOP);
						}
						else {
							SDL_SetWindowFullscreen(window, win_flgs);
						}
						break;
					default:
						break;
				}
			}
			else if (event.type == SDL_QUIT) {
				running = 0;
			}
		}


		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ..:: Drawing code (in render loop) :: ..
		glUseProgram(Shader_array[0]);
		glBindVertexArray(VAO_array[0]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);	

		SDL_GL_SwapWindow(window);
	}

exit:
	return (0);
}

/* ================= DEFINE FUNCTIONS ================ */

/*
 * Initialize all necessary structs and
 * objects.
 *
 * Returns: 0 if successfull or -1 if
 * 	an error occurred
 */
int init()
{
	printf("Init sdl...");
	if(SDL_Init(XSDL_INIT_EVERYTHING) < 0) {
		printf("\n[!] SDL could not initialize! (%s)\n", SDL_GetError());
		return (-1);
	}
	SDL_version compiled;
	SDL_VERSION(&compiled);
	printf("done. (%d.%d.%d)\n", compiled.major, compiled.minor, compiled.patch);

	printf("Setup window...");
	if((window = init_window()) == NULL) {
		printf("\n[!] Window could not be created! (%s)\n", SDL_GetError());
		return (-1);
	}
	printf("done.\n");

	printf("Init glcontext...");
	if((gl_ctx = init_glcontext(window)) == NULL) {
		printf("\n[!] GL-Context could not be created!\n");
		return (-1);
	}
	printf("done. (%s)\n", glGetString(GL_VERSION)); 

	printf("Init uicontext...");
	if((ui_ctx = XSDL_CreateUIContext(window)) == NULL) {
		printf("\n[!] UI-Context could not be created!\n");
		return (-1);
	}
	printf("done.\n");

	return(0);
}

/*
 * Initialize the window and configure basic settings 
 * like the title, minimal window size and window-icon.
 *
 * Returns: Window-Pointer or NULL if an error occurred
 */
XSDL_Window *init_window()
{
	/* Create and initialize the window */
	XSDL_Window *win = SDL_CreateWindow("Vasall", 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			win_w, win_h, 
			win_flgs);

	if(win == NULL) return (NULL);

	assert(win);

	return(win);
}

/*
 * This function will initialize the OpenGL-context
 * and set the render-flags.
 *
 * @win: Pointer to underlying window
 *
 * Returns: GlContext or NULL if an error occurred
 */
XSDL_GLContext *init_glcontext(SDL_Window *win)
{
	XSDL_GLContext ctx = SDL_GL_CreateContext(win);

	return(ctx);
}

/*
 * Initialize the GUI and place all elements.
 *
 * @ctx: The ui-context to attach elements to
 */
void init_gui(XSDL_UIContext *ctx)
{
	XSDL_Node *rootnode = ctx->root;

	/* Create the menu-sceen */
	XSDL_CreateWrapper(rootnode, "mns", 0, 0, -100, -100);
	XSDL_Node *mns_form = XSDL_CreateWrapper(XSDL_Get(rootnode, "mns"), 
			"mns_form", -1, -1, 400, 380);

	XSDL_CreateWrapper(mns_form, "mns_title",
			0, 0, 400, 80);
	XSDL_Rect body0 = {50, 14, 300, 52};
	XSDL_CreateText(XSDL_Get(rootnode, "mns_title"), "label0", &body0,
			"VASALL", &XSDL_WHITE, 2, 0);

	XSDL_Rect body1 = {40, 106, 320, 24};
	XSDL_CreateText(mns_form, "mns_user_label", &body1,"Email:", 
			&XSDL_WHITE, 1, XSDL_TEXT_LEFT);
	XSDL_CreateInput(mns_form, "mns_user", 40, 130, 320, 40, "");

	XSDL_Rect body2 = {40, 186, 320, 24};
	XSDL_CreateText(mns_form, "mns_user_label", &body2, "Password:", 
			&XSDL_WHITE, 1, XSDL_TEXT_LEFT);
	XSDL_CreateInput(mns_form, "mns_pswd", 40, 210, 320, 40, "");

	XSDL_CreateButton(XSDL_Get(rootnode, "mns_form"), "mns_login", 
			40, 280, 320, 40, "Login");

	XSDL_Color mns_form_bck_col = {0x23, 0x23, 0x23, 0xff};
	short mns_form_corners[] = {5, 5, 5, 5};
	XSDL_ModStyle(mns_form, XSDL_STY_VIS, &one);
	XSDL_ModStyle(mns_form, XSDL_STY_BCK, &one);
	XSDL_ModStyle(mns_form, XSDL_STY_BCK_COL, &mns_form_bck_col);
	XSDL_ModStyle(mns_form, XSDL_STY_COR_RAD, &mns_form_corners);

	XSDL_Color mns_title_bck_col = {0xd3, 0x34, 0x5a, 0xff};
	short mns_title_cor[] = {5, 5, 0, 0};
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_BCK_COL, &mns_title_bck_col);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_title"), XSDL_STY_COR_RAD, &mns_title_cor);

	/* XSDL_BindEvent(XSDL_Get(rootnode, "mns_login"), XSDL_EVT_MOUSEDOWN, &try_login); */

	/* Create the game-sceen */
	XSDL_CreateWrapper(rootnode, "gms", 0, 0, -100, -100);
	XSDL_Color gms_bck_col = {0x47, 0x2d, 0x5c, 0xff};
	XSDL_ModStyle(XSDL_Get(rootnode, "gms"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms"), XSDL_STY_BCK_COL, &gms_bck_col);

	XSDL_CreateWrapper(XSDL_Get(rootnode, "gms"), "gms_stats", -1, 5, 780, 35);
	XSDL_Color gms_stats_bck_col = {0x23, 0x25, 0x30, 0xff};
	short gms_stats_cor[] = {6, 6, 6, 6};
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_VIS, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_BCK, &one);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_BCK_COL, &gms_stats_bck_col);
	XSDL_ModStyle(XSDL_Get(rootnode, "gms_stats"), XSDL_STY_COR_RAD, &gms_stats_cor);

	XSDL_ModFlag(XSDL_Get(rootnode, "gms"), XSDL_FLG_ACT, &zero);

	XSDL_ShowNodes(rootnode);
}

#if DEBUG_TEST

/*
 * Load all necessary resources, like fonts, sprites
 * and more.
 *
 * Returns: 0 on success and -1 if an error occurred
 */
int init_resources()
{
	char cwd[256];
	char exe_dir[256];
	char path[512];
	int i = 0;

	readlink("/proc/self/exe", cwd, 256);
	for(i = strlen(cwd); i >= 0; i--)
		if(cwd[i] == '/')
			break;
	memcpy(exe_dir, cwd, i);

	printf("Directory: %s\n", exe_dir);

	sprintf(path, "%s/%s", exe_dir, "res/mecha.ttf");	
	if(XSDL_LoadFont(path, 24) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/aller.ttf");
	if(XSDL_LoadFont(path, 16) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/editundo.ttf");
	if(XSDL_LoadFont(path, 48) < 0)
		goto loadfailed;

	sprintf(path, "%s/%s", exe_dir, "res/mns_bck.png");
	if(XSDL_LoadImage(renderer, path) < 0)
		goto loadfailed;

	return(0);

loadfailed:
	return(-1);
}

#endif
