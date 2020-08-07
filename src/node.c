#include "node.h"
#include "window.h"

const ui_constr_ent UI_POS_CONSTR_NULL = {
	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0},
	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0},
	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0},

	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0},
	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0},
	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0}
};

const ui_constr_ent UI_POS_CONSTR_NULL = {
	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0},
	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0},
	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0},

	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0},
	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0},
	{UI_CONSTR_NONE, 0, UI_CONSTR_PX, 0}
};

const ui_node_flags UI_NULL_FLAGS = {
	1, 0, 0, 0
};

const ui_node_style UI_NULL_STYLE = {
	1, 0, {0,0,0,0}, 0, {0,0,0,0}, {0,0,0,0}
};

const ui_node_events UI_NULL_EVENTS = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

const float STD_CORNERS[18] = {
	1.0,  1.0, 0.0, -1.0,  1.0, 0.0, -1.0, -1.0, 0.0,
	-1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 1.0,  1.0, 0.0
};

const float STD_UV[12] = {
	1.0, 0.0, 0.0, 0.0, 0.0, 1.0,
	0.0, 1.0, 1.0, 1.0, 1.0, 0.0
};


static int ui_add_child(ui_node *n, ui_node *c)
{
	short i;

	for(i = 0; i < UI_CHILD_NUM; i++) {
		if(n->children[i] == NULL) {
			n->children[i] = c;
			n->child_num++;
			return 0;
		}
	}

	return -1;
}

static int ui_init_wrapper(ui_node *n)
{
	n->flags = WRAPPER_FLAGS;
	n->style = WRAPPER_STYLE;
	return 0;
}

static int ui_init_text(ui_node *n)
{
	n->style = TEXT_STYLE;
	n->render = &TEXT_RENDER;
	return 0;
}

extern ui_node *ui_add(ui_tag tag, ui_node *par, void *ele, char *id)
{
	int i;
	ui_node *node;

	/* Allocate memory for the node */
	if(!(node = malloc(sizeof(ui_node))))
		return NULL;

	/* Set the tag and the element of the node */
	node->tag = tag;
	node->element = ele;

	/* Copy the id to the node */
	strcpy(node->id, id);

	/* Clear all child-slots */
	node->child_num = 0;
	for(i = 0; i < CHILD_NUM; i++)
		node->children[i] = NULL;

	/* Use default position- and size-constraints */
	node->pos_constr =  UI_POS_CONSTR_NULL;
	node->size_constr = UI_SIZE_CONSTR_NULL;

	/* Use default flags, style and events */
	node->flags =  NULL_FLAGS;
	node->style =  NULL_STYLE;
	node->events = NULL_EVENTS;

	node->render = NULL;
	node->del = NULL;

	memset(&node->body,  0, sizeof(rect_t));
	memset(&node->rel_body, 0, sizeof(rect_t));

	node->vao = 0;
	for(i = 0; i < 2; i++)
		node->bao[i] = 0;
	node->tex = 0;
	node->surf = NULL;

	if(par != NULL) {
		node->layer = par->layer + 1;
		node->parent = par;

		/* Attach node as child to parent */
		if(ui_add_child(par, node) < 0)
			goto err_free_node;
	}
	else {
		node->layer = 0;
		node->parent = NULL;
	}

	/* Call the custom initialization-function for the node-type */
	switch(tag) {
		case UI_WRAPPER: ui_init_wrapper(node); break;
	}

	/* Adjust the size of the node */
	ui_adjust(node);

	/* Prerender the node */
	ui_update(node);
	return node;

err_free_node:
	free(node);
	return NULL;
}


static void ui_del_node(ui_node *n, void *data)
{
	int i;

	if(data){/* Prevent warning for not using parameters */}

	if(n->del != NULL)
		n->del(n, NULL);

	if(n->surf != NULL) {
		SDL_FreeSurface(n->surf);		
		glDeleteTextures(1, &n->tex);
		glDeleteBuffers(2, n->bao);
		glDeleteVertexArrays(1, &n->vao);
	}

	free(n);
}

extern void ui_remv(ui_node *n)
{
	uint8_t flg;
	ui_node *par;
	short i;
	short j = 0;

	/* Remove node from child-list of parent node */
	par = n->parent;
	if(par != NULL) {
		for(i = 0; i < par->child_num; i++) {
			if(par->children[i] == n) {
				/* Reorder child-list */
				for(i++; i < par->child_num; i++) {
					par->children[i - 1] = par->children[i];
				}
				par->child_num--;
				break;
			}
		}
	}

	/* Free allocated memory and delete surfaces and textures */
	flg = UI_DOWN_POST | UI_DOWN_ALL;
	ui_down(n, &ui_del_node, NULL, flg);
}


extern ui_node *ui_get(ui_node *n, char *id)
{
	short i;
	ui_node *res;

	if(strcmp(n->id, id) == 0)
		return n;

	for(i = 0; i < n->child_num; i++) {
		if((res = ui_get(n->children[i], id)) != NULL)
			return res;
	}

	return NULL;
}


extern void ui_down(ui_node *n, ui_fnc fnc, void *data, uint8_t flg)
{
	short i;

	/* Show only active branches be processed */
	if((flg & UI_DOWN_ALL) != UI_DOWN_ALL && n->flags.active == 0)
		return;

	/* Preorder */
	if((flg & UI_DOWN_PRE) == UI_DOWN_PRE)
		func(n, data);

	/* Apply function to all child nodes */
	for(i = 0; i < n->child_num; i++)
		ui_down(n->children[i], func, data, flg);

	/* Postorder */
	if((flg & UI_DOWN_POST) == UI_DOWN_POST)
		func(n, data);
}


extern void ui_up(ui_node *n, ui_fnc fnc, void *data)
{
	ui_node *ptr = n;

	while(ptr) {
		func(ptr, data);
		ptr = ptr->parent;
	}
}


int ui_set_constr(ui_node *n, ui_constr_type type, ui_constr_algn algn,
		ui_constr_mod mod, ui_constr_mask mask, float val,
		ui_constr_unit unit, ui_constr_rel rel)
{
	ui_constr_ent *constr;

	if(type == UI_CONSTR_POS)
		constr = &n->pos_constr[algn][mod];
	else if(type == UI_CONSTR_SIZE)
		constr = &n->size_constr[algn][mod];

	constr->mask = mask;
	constr->val = val;
	constr->unit = unit;
	constr->rel = rel;
	return 0;
}


static void ui_prerender_node(ui_node *n, ui_node *rel, char flg)
{
	int i;
	rect_t body;
	ui_node_style *style;

	if(n->flags.active == 0)
		return;

	if(flg == 1 && n->surf != NULL)
		return;

	if(flg == 0)
		flg = 1;

	memcpy(&body, &n->rel_body, sizeof(rect));

	style = &n->style;

	/* Ignore if the node should not be visible */
	if(style->vis == 0)
		goto next;

	/* Render border if enabled */
	if(style->bor > 0) {
		sdl_fill_rounded(rel->surf,
				body.x,
				body.y,
				body.w,
				body.h,
				style->bor_col,
				style->cor_rad);
	}

	/* Render background if enabled */
	if(style->bck > 0) {
		short w = style->bor;
		sdl_fill_rounded(rel->surf,
				body.x + w,
				body.y + w,
				body.w - (2 * w),
				body.h - (2 * w),
				style->bck_col,
				style->cor_rad);
	}

	/* Run custum render-function */
	if(n->render != NULL)
		n->render(n, rel);

next:
	for(i = 0; i < n->child_num; i++)
		ui_prerender_node(n->children[i], rel, flg);
}

extern void ui_update(ui_node *n)
{
	ui_node *s = n;

	/* Search for a render-node with an attached surface */
	while(s->parent != NULL) {
		if(s->surf != NULL)
			break;

		s = s->parent;
	}

	/* If no render-node has been found */
	if(s->surf == NULL)
		return;

	/* Clear surface */
	SDL_LockSurface(s->surf);
	memset(s->surf->pixels, 0, s->surf->h * s->surf->pitch);
	SDL_UnlockSurface(s->surf);

	/* Recursivly draw all nodes onto surface */
	ui_prerender_node(s, s, 0);

	/* Update the texture of the node */
	glBindTexture(GL_TEXTURE_2D, s->tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, s->surf->w, s->surf->h,
			GL_RGBA, GL_UNSIGNED_BYTE, s->surf->pixels);
}


extern void ui_render(ui_node *n)
{
	if(n->vao != 0 && n->tex != 0 && n->surf != NULL) {
		glBindVertexArray(n->vao);
		glUseProgram(window.shader);

		glBindTexture(GL_TEXTURE_2D, n->tex);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glUseProgram(0);
		glBindVertexArray(0);
	}
}


extern void ui_show_node(ui_node *node, void *data)
{
	int i;

	if(data){/* Prevent warning for not using parameters */}

	for(i = 0; i < node->layer; i++)
		printf("  ");

	printf("%s", node->strid);

	if(node->tex != 0 && node->surf != NULL)
		printf(" R ");
	else
		printf(" ");

	if(node->flags.active == 1) 
		printf("ON");
	else
		printf("OFF");

	printf("\n");
}


extern void ui_show(ui_node *n)
{
	printf("\nNODE-TREE:\n");
	ui_down(n, &ui_show_node, NULL, 0);
}

static void ui_adjust_pos(ui_constr *constr, rect_t *out_abs, rect_t *proc_rel,
		rect_t *rel, rect_t *rend)
{
	char i;
	char j;
	ui_constr_mask mask;
	float val;
	ui_constr_unit unit;
	ui_constr_rel rel;

	short size_val;
	short cmp_val;
	short tmp[2] = {0, 0};
	short pos;

	proc_rel.x = 0;
	proc_rel.y = 0;
	proc_rel.w = 100;
	proc_rel.h = 100;

	out_abs.x = 0;
	out_abs.y = 0;
	out_abs.w = 100;
	out_abs.h = 100;

	for(i = 0; i < 2; i++) {
		size_val = (i == 0) ? proc_rel.w : proc_rel.h; 

		for(j = 0; j < 3; j++) {
			if((mask = constr->ent[i][j].mask) == UI_CONSTR_NONE)
				continue;
			
			val = constr->ent[i][j].val;
			unit = constr->ent[i][j].unit;
			rel = constr->ent[i][j].rel;

			/* Get relative position and size for axis */
			switch(rel) {
				case UI_CONSTR_REL:
					if(i == 0) {
						tmp[0] = rel->x;
						tmp[1] = rel->w;
					}
					else {
						tmp[0] = rel->y;
						tmp[1] = rel->h;
					}
					break;

				case UI_CONSTR_ABS:
					if(i == 0) {
						tmp[0] = rend->x;
						tmp[1] = rend->w;
					}
					else {
						tmp[0] = rend->y;
						tmp[1] = rend->h;
					}
					break;

				case UI_CONSTR_WIN:
					if(i == 0) {
						tmp[0] = 0;
						tmp[1] = window.win_w;
					}
					else {
						tmp[0] = 0;
						tmp[1] = window.win_h;
					}
					break;
			}

			/* Convert value depending on unit */
			switch(unit) {
				case UI_CONSTR_PX:
					cmp_val = (short)val;
					break;

				case UI_CONSTR_PCT:
					cmp_val = (short)((float)tmp[1] * val);
					break;
			}

			/* Calculate position depending on alignment-mask */
			switch(mask) {
				case UI_CONSTR_AUTO:
					cmp_val = (tmp[1] / 2) - (size_val / 2);
					break;

				case UI_CONSTR_SET:
				case UI_CONSTR_LEFT:
				case UI_CONSTR_TOP:
					cmp_val = cmp_val;
					break;

				case UI_CONSTR_RIGHT:
				case UI_CONSTR_BOTTOM:
					cmp_val = tmp[1] - cmp_val;
					break;
			}

			/* Set or limit value */
			switch(j) {
				case 0:
					pos = cmp_val;
					break;
				case 1:
					if(pos > cmp_val)
						pos = cmp_val;
					break;
				case 2:
					if(pos < cmp_val)
						pos = cmp_val;
					break;
			}

			/* Write position into buffers */
			if(i == 0) {
				out_rel.x = pos;
				out_abs.x = tmp[0] + pos;
			}
			else {
				out_rel.y = pos;
				out_abs.y = tmp[0] + pos;
			}
		}
	}
}

static void ui_adjust_node(ui_node *n, void *data)
{
	int i, value;
	
	ui_node *par;
	ui_node *rend;

	rect_t win, *rel;

	/* Create shortcuts for the parent- and render-node */
	par = n->parent;
	rend = (ui_node *)data;

	/* The window-size and default position */
	win.x = 0;
	win.y = 0;
	win.w = window.win_w;
	win.h = window.win_h;

	/* Calculate the size of the parent-node */
	if(par == NULL) memcpy(rel, &win, sizeof(rect_t));
	else memcpy(rel, &par->body, sizeof(rect_t));

	/* Set position */
	ui_adjust_pos(&n->pos_constr, &n->body, &n->rel_body, rel, &rend->body)
}

extern void ui_adjust(ui_node *n)
{
	ui_node *rel = n;

	/* Search for a render-node with an attached surface */
	while(rel->parent != NULL) {
		if(rel->surf != NULL)
			break;

		rel = rel->parent;
	}

	/* If no render-node has been found */
	if(rel->surf == NULL)
		rel = NULL;

	ui_run_down(n, &ui_adjust_node, rel, 0);
}

static int ui_init_buffers(ui_node *n)
{
	/* Generate a new vao and bind it */
	glGenVertexArrays(1, &n->vao);
	glBindVertexArray(n->vao);

	/* Create two buffers */
	glGenBuffers(2, n->bao);

	/* Register the vertex-positions */
	glBindBuffer(GL_ARRAY_BUFFER, n->bao[0]);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float),
			STD_CORNERS, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	/* Register new texture-coordinates */
	glBindBuffer(GL_ARRAY_BUFFER, n->bao[1]);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float),
			STD_UV, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	/* Unbind the vertex-array-object */
	glBindVertexArray(0);
	return 0;
}

static int ui_init_surf(ui_node *n)
{
	int w, h;
	w = n->body.w;
	h = n->body.h;

	/* Create the render-surface */
	if(!(n->surf = SDL_CreateRGBSurface(0, w, h, 32, SDL_R_MASK, 
					SDL_G_MASK, SDL_B_MASK, SDL_A_MASK)))
		return -1;

	SDL_LockSurface(n->surf);
	memset(n->surf->pixels, 0xff, n->surf->pitch * n->surf->h);
	SDL_UnlockSurface(n->surf);
	return 0;
}

static int ui_init_tex(ui_node *n)
{
	int w = n->abs.w;
	int h = n->abs.h;

	/* Create new texture */
	glGenTextures(1, &n->tex);
	glBindTexture(GL_TEXTURE_2D, n->tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, 
			GL_UNSIGNED_BYTE, n->surf->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	return 0;
}

extern int ui_enable_tex(ui_node *n)
{
	if(ui_init_surf(n) < 0)
		return -1;

	if(ui_init_buffers(n) < 0)
		return -1;

	if(ui_init_tex(n) < 0)
		return -1;

	ui_update_tex(n);
	return 0;
}

extern void ui_update_tex(ui_node *n)
{
	float vtx[18];
	float cw, ch, x, y, w, h;

	memset(vtx, 0, sizeof(float) * 18);

	cw = (float)window.win_w / 2.0;
	ch = (float)window.win_h / 2.0;

	x = (float)n->body.x;
	y = (float)n->body.y;
	w = (float)n->body.w;
	h = (float)n->body.h;

	/* Top-Right */
	vtx[0] = ((x + w) / cw) - 1.0;
	vtx[1] = 1.0 - (y / ch);
	/* Top-Left */
	vtx[3] = (x / cw) - 1.0;
	vtx[4] = 1.0 - (y / ch);
	/* Bottom-Left */
	vtx[6] = (x / cw) - 1.0;
	vtx[7] = 1.0 - ((y + h) / ch);

	/* Bottom-Left */
	vtx[9] = (x / cw) - 1.0;
	vtx[10] = 1.0 - ((y + h) / ch);
	/* Bottom-Right */
	vtx[12] = ((x + w) / cw) - 1.0;
	vtx[13] = 1.0 - ((y + h) / ch);
	/* Top-Right */
	vtx[15] = ((x + w) / cw) - 1.0;
	vtx[16] = 1.0 - (y / ch);

	glBindVertexArray(n->vao);
	glBindBuffer(GL_ARRAY_BUFFER, n->bao[0]);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), vtx, GL_STATIC_DRAW);
	glBindVertexArray(0);
}

extern void ui_mod_flag(ui_node *n, short flg, void *val)
{
	ui_node_flags *flags;

	flags = (ui_node_flags *)&n->flags;
	switch(flg) {
		case(FLG_ACT):
			if(flags->active == *((uint8_t *)val)) {
				return;
			}

			flags->active = *((uint8_t *)val);

			ui_prerender(n);
			win_build_pipe();
			break;

		case(FLG_EVT):
			flags->procevt = *((uint8_t *)val);
			break;

		case(FLG_CUR):
			flags->cur = *(uint8_t *)val;
			break;
	}
}

void ui_mod_style(ui_node *n, short opt, void *val)
{
	ui_node_style *style = &n->style;

	switch(opt) {
		case(STY_VIS):
			style->vis = *((char *)val);
			break;

		case(STY_BCK):
			style->bck = *((char *)val);
			break;

		case(STY_BCK_COL):
			style->bck_col = *((SDL_Color *)val);
			break;

		case(STY_BOR):
			style->bor = *((short *)val);
			break;

		case(STY_BOR_COL):
			style->bor_col = *((SDL_Color *)val);
			break;

		case(STY_COR_RAD):
			memcpy(&style->cor_rad, val, 4 * sizeof(short));
			break;

		case(STY_INS):
			memcpy(&style->ins, val, 4 * sizeof(short));
			break;
	}

	ui_prerender(n);
}

void ui_bind_event(ui_node *n, short evt, ui_node_callback ptr)
{
	ui_node_events *events = &n->events;

	switch(evt) {
		case(EVT_MOUSEDOWN): events->mousedown = ptr; break;
		case(EVT_MOUSEUP): events->mouseup = ptr; break;
		case(EVT_HOVER): events->hover = ptr; break;
		case(EVT_KEYDOWN): events->keydown = ptr; break;
		case(EVT_KEYUP): events->keyup = ptr; break;
	}
}
