#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "gui.h"

/*
 * Initialize the GUI and place all elements.
 *
 * @ctx: The context to create
*/
void init_gui(XSDL_Context *ctx)
{
	XSDL_Node *rootnode = ctx->root;

	/* Create the menu-sceen */
	XSDL_CreateWrapper(rootnode, "menu", 
			0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	XSDL_CreateWrapper(XSDL_Get(rootnode, "menu"), "mns_form",
			200, 125, 400, 350);


	SDL_Rect *body_mem;
	SDL_Color *col_mem;

	SDL_Rect body = {250, 206, 300, 24};
	SDL_Color col = {0xff, 0xff, 0xff};
	body_mem = (SDL_Rect *)malloc(sizeof(body));
	memcpy(body_mem, &body, sizeof(body));
	col_mem = (SDL_Color *)malloc(sizeof(col));
	memcpy(col_mem, &col, sizeof(col));
	XSDL_CreateText(XSDL_Get(rootnode, "mns_form"), "label1", body_mem,
		"Email:", col_mem, 1, XSDL_TEXT_LEFT);
	XSDL_CreateInput(XSDL_Get(rootnode, "mns_form"), "mns_user", 
			250, 230, 300, 40, "");

	SDL_Rect body1 = {250, 286, 300, 24};
	SDL_Color col1 = {0xff, 0xff, 0xff};
	body_mem = (SDL_Rect *)malloc(sizeof(body1));
	memcpy(body_mem, &body1, sizeof(body1));
	col_mem = (SDL_Color *)malloc(sizeof(col1));
	memcpy(col_mem, &col1, sizeof(col1));
	XSDL_CreateText(XSDL_Get(rootnode, "mns_form"), "label2", body_mem,
		"Password:", col_mem, 1, XSDL_TEXT_LEFT);
	XSDL_CreateInput(XSDL_Get(rootnode, "mns_form"), "mns_pswd", 
			250, 310, 300, 40, "");
	
	XSDL_CreateButton(XSDL_Get(rootnode, "mns_form"), "mns_login", 
			250, 380, 300, 40, "Login");

	uint8_t vis = 1;
	uint8_t bck = 1;
	SDL_Color bck_col = {0xff, 0x00, 0x00};
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_form"), XSDL_STY_VIS, &vis);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_form"), XSDL_STY_BCK, &bck);
	XSDL_ModStyle(XSDL_Get(rootnode, "mns_form"), XSDL_STY_BCK_COL, &bck_col);
}

