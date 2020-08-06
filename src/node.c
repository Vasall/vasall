#include "node.h"
#include "window.h"

const ui_node_flags NULL_FLAGS = {
	1, 0, 0, 0
};

const ui_node_style NULL_STYLE = {
	1, 0, {0,0,0,0}, 0, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, 0, 0
};

const ui_node_events NULL_EVENTS = {
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

static int ui_init_wrapper(struct ui_node *n)
{
	n->flags = WRAPPER_FLAGS;
	n->style = WRAPPER_STYLE;
	return 0;
}

static int ui_init_text(struct ui_node *n)
{
	n->flags = NULL_FLAGS;
	n->style = TEXT_STYLE;
	n->render = &TEXT_RENDER;
	return 0;
}

extern ui_node *ui_add(enum ui_tag tag, struct ui_node *par, void *ele,
		char *id, rect_t rect)
{
	int i;
	struct ui_node *node;

	if(!(node = malloc(sizeof(struct ui_node))))
		return NULL;

	strcpy(node->strid, id);

	node->child_num = 0;
	for(i = 0; i < CHILD_NUM; i++)
		node->children[i] = NULL;

	node->tag = tag;
	node->element = ele;

	node->flags = NULL_FLAGS;
	node->style = NULL_STYLE;
	node->events = NULL_EVENTS;

	node->render = NULL;
	node->del = NULL;

	for(i = 0; i < 6; i++)
		node->pos_constr[i] = NULL;

	for(i = 0; i < 6; i++)
		node->size_constr[i] = NULL;

	memset(&node->rel,  0, sizeof(rect_t));
	memset(&node->body, 0, sizeof(rect_t));
	memset(&node->abs,  0, sizeof(rect_t));

	node->vao = 0;
	for(i = 0; i < 2; i++)
		node->bao[i] = 0;
	node->tex = 0;
	node->surf = NULL;

	if(par != NULL) {
		int childp = -1;

		node->layer = par->layer + 1;
		node->parent = par;

		for(i = 0; i < CHILD_NUM; i++) {
			if(par->children[i] == NULL) {
				childp = i;
				break;
			}
		}

		if(childp == -1)
			goto err_free_node;

		par->children[childp] = node;
		par->child_num++;
	}
	else {
		node->layer = 0;
		node->parent = NULL;
		window.root = node;
		node->style.vis = 0;
	}

	if(rect != NULL) {
		ui_set_pos_constr(node, "ver",  CONSTR_SET,
				CONSTR_HORI, 0, CONSTR_LEFT,
				rect.x, CONSTR_PX);

		ui_set_pos_constr(node, "hor", CONSTR_SET,
				CONSTR_VERT, 0, CONSTR_LEFT,
				rect.y, CONSTR_PX);

		ui_set_size_constr(node, "s_h", CONSTR_SET,
				CONSTR_HORI, 0, rect.w, CONSTR_PX);

		ui_set_size_constr(node, "s_v", CONSTR_SET,
				CONSTR_VERT, 0, rect.h, CONSTR_PX);
	}

	ui_adjust(node);
	
	if(node->parent == NULL)
		ui_enable_tex(node);

	switch(tag) {
		case UI_WRAPPER: ui_init_wrapper(node); break;
	}

	ui_prerender(node);
	return node;

err_free_node:
	free(node);
	return NULL;
}

static void ui_delete_node(ui_node *n, void *data)
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

	for(i = 0; i < 6; i++) {
		free(n->pos_constr[i]);
		free(n->size_constr[i]);
	}
	free(n);
}

extern void ui_remv(ui_node *n)
{
	uint8_t flg = RUN_DOWN_AFT | RUN_DOWN_ALL;
	ui_run_down(n, &ui_delete_node, NULL, flg);
}

static ui_node *ui_search_node(ui_node *n, char *strid)
{
	int i;
	ui_node *res;

	if(strcmp(n->strid, strid) == 0)
		return n;

	for(i = 0; i < n->child_num; i++) {
		if((res = ui_search_node(n->children[i], strid)))
			return res;
	}

	return NULL;
}

extern ui_node *ui_get(char *strid)
{
	return ui_search_node(window.root, strid);
	
}

extern int ui_set_pos_constr(ui_node *n, char *id, ui_constr_type type,
		ui_constr_axis axis, uint8_t rel,
		ui_constr_align align, float val,
		ui_constr_unit unit)
{
	int idx = -1, i;
	ui_pos_constr *constr;

	if(align){/* Prevent warning for not using parameters */}

	for(i = 0; i < 6; i++) {
		if(n->pos_constr[i] == NULL) {
			idx = i;
			break;
		}
	}

	if(idx < 0)
		return -1;

	if(!(constr = malloc(sizeof(ui_pos_constr))))
		return -1;

	strcpy(constr->id, id);
	constr->type = type;
	constr->axis = axis;
	constr->rel = rel;
	constr->type = type;
	constr->value = val;
	constr->unit = unit;

	n->pos_constr[idx] = constr;

	ui_adjust(n);
	return 0;
}

extern void ui_del_pos_constr(ui_node *n, char *id)
{
	int i;

	if(n == NULL || id == NULL)
		return;

	for(i = 0; i < 6; i++) {
		if(n->pos_constr[i] != NULL) {
			free(n->pos_constr[i]);
			n->pos_constr[i] = NULL;
		}
	}
}

extern int ui_set_size_constr(ui_node *n, char *id, ui_constr_type type,
		ui_constr_axis axis, uint8_t rel, float val, 
		ui_constr_unit unit)
{
	int idx = -1, i;
	ui_size_constr *constr;

	for(i = 0; i < 6; i++) {
		if(n->size_constr[i] == NULL) {
			idx = i;
			break;
		}
	}

	if(idx < 0) 
		return -1;

	if(!(constr = malloc(sizeof(ui_size_constr))))
		return -1;

	strcpy(constr->id, id);
	constr->type = type;
	constr->axis = axis;
	constr->rel = rel;
	constr->value = val;
	constr->unit = unit;

	n->size_constr[idx] = constr;

	ui_adjust(n);
	return 0;
}

extern void ui_del_size_constr(ui_node *n, char *id)
{
	int i;

	if(n == NULL || id == NULL)
		return;

	for(i = 0; i < 6; i++) {
		if(n->size_constr[i] != NULL) {
			free(n->size_constr[i]);
			n->size_constr[i] = NULL;
		}
	}
}

static void ui_prerender_node(ui_node *n, ui_node *rel, char flg)
{
	int i;
	SDL_Rect rend;
	ui_node_style *style;

	if(n == NULL || rel == NULL)
		return;

	if(n->flags.active == 0)
		return;

	if(flg && n->surf != NULL)
		return;

	if(flg == 0)
		flg = 1;

	if(n->surf != NULL && n->tex != 0) {
		SDL_LockSurface(n->surf);
		memset(n->surf->pixels, 0, n->surf->h * n->surf->pitch);
		SDL_UnlockSurface(n->surf);
	}

	memcpy(&rend, &n->abs, sizeof(SDL_Rect));
	rend.x -= rel->abs.x;
	rend.y -= rel->abs.y;

	style = &n->style;

	/* Ignore if the node should not be visible */
	if(style->vis != 0) {	
		/* Render border if enabled */
		if(style->bor > 0) {
			short w = style->bor;
			sdl_fill_rounded(rel->surf,
					rend.x - w,
					rend.y - w,
					rend.w + (w * 2),
					rend.h + (w * 2),
					style->bor_col,
					style->cor_rad);
		}

		/* Render background if enabled */
		if(style->bck > 0) {
			sdl_fill_rounded(rel->surf,
					rend.x,
					rend.y,
					rend.w,
					rend.h,
					style->bck_col,
					style->cor_rad);
		}
	}

	/* Run custum render-function */
	if(n->render != NULL) {
		n->render(n, rel);
	}

	if(n->surf != NULL && n->tex != 0) {
		for(i = 0; i < n->child_num; i++) {
			ui_prerender_node(n->children[i], n, flg);
		}
		/* Update the texture of the node */
		glBindTexture(GL_TEXTURE_2D, n->tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, n->surf->w,
				n->surf->h, GL_RGBA, GL_UNSIGNED_BYTE,
				n->surf->pixels);
	}
	else {
		/* Draw the child nodes on the relative-surface */
		for(i = 0; i < n->child_num; i++) {
			ui_prerender_node(n->children[i], rel, flg);
		}
	}
}

extern void ui_prerender(ui_node *n)
{
	ui_node *start = n;

	while(start->parent != NULL) {
		if(start->surf != NULL)
			break;

		start = start->parent;
	}

	ui_prerender_node(start, start, 0);
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

extern void ui_run_down(ui_node *n, void (*func)(ui_node *n, void *d),
		void *data, uint8_t flg)
{
	int i;

	/* Skip with XSDL_RECDOWN_ALL enabled */
	if((flg & 1) == 0 && n->flags.active == 0)
		return;

	/* Skip with XSDL_RECDOWN_AFT enabled */
	if(((flg >> 1) & 1) == 0)
		func(n, data);

	/* Apply function to all child nodes */
	for(i = 0; i < n->child_num; i++) {
		ui_run_down(n->children[i], func, data, flg);
	}

	/* Use with XSDL_RECDOWN_AFT enabled */
	if(((flg >> 1) & 1) == 1)
		func(n, data);
}

extern void ui_run_up(ui_node *n, void (*func)(ui_node *n, void *d),
		void *data)
{
	ui_node *ptr = n;

	while(ptr != NULL) {
		func(ptr, data);
		ptr = ptr->parent;
	}
}

extern void ui_show_node(ui_node *node, void *data)
{
	int i;

	if(data){/* Prevent warning for not using parameters */}

	for(i = 0; i < node->layer; i++) {
		printf("  ");
	}

	printf("%s", node->strid);

	if(node->tex != 0 && node->surf != NULL) {
		printf(" R ");
	}
	else {
		printf(" ");
	}

	if(node->flags.active == 1) {
		printf("ON");
	}
	else {
		printf("OFF");
	}

	printf("\n");
}

extern void ui_show(ui_node *n)
{
	printf("\nNODE-TREE:\n");
	ui_run_down(n, &ui_show_node, NULL, 0);
}

static void ui_adjust_node(ui_node *n, void *data)
{
	int i, value;
	ui_node *par;
	ui_node_style *style;
	SDL_Rect r, p, *use, *rel, *body, *abs;
	ui_pos_constr *pos_c;
	ui_size_constr *size_c;

	if(data){/* Prevent warning for not using parameters */}

	par = n->parent;
	style = &n->style;

	rel = &n->rel;
	body = &n->body;
	abs = &n->abs;

	r.x = 0;
	r.y = 0;
	r.w = window.win_w;
	r.h = window.win_h;

	/* Calculate the size of the parent-node */
	if(par == NULL) memcpy(&p, &r, sizeof(SDL_Rect));
	else memcpy(&p, &par->body, sizeof(SDL_Rect));

	memcpy(rel, &p, sizeof(SDL_Rect));

	for(i = 0; i < 6; i++) {
		if(!(pos_c = n->pos_constr[i]))
			continue;

		/* Determite the relative element */
		if(pos_c->rel == 1) use = &r;
		else use = &p;

		if(pos_c->axis == CONSTR_HORI) {
			if(pos_c->unit == CONSTR_PCT) {
				value = (int)(use->w * pos_c->value);
			}
			else {
				value = (int)pos_c->value;
			}

			if(pos_c->type == CONSTR_MIN) {
				if(rel->x < value) {
					rel->x = value;
				}
			}
			else if(pos_c->type == CONSTR_MAX) {
				if(rel->x > value) {
					rel->x = value;
				}
			}
			else {
				rel->x = use->x + value;
			}
		}
		else if(pos_c->axis == CONSTR_VERT) {
			if(pos_c->unit == CONSTR_PCT) {
				value = (int)(use->h * pos_c->value);
			}
			else {
				value = (int)pos_c->value;
			}

			if(pos_c->type == CONSTR_MIN) {
				if(rel->y < value) {
					rel->y = value;
				}
			}
			else if(pos_c->type == CONSTR_MAX) {
				if(rel->y > value) {
					rel->y = value;
				}
			}
			else {
				rel->y = use->y + value;
			}
		}
	}

	for(i = 0; i < 6; i++) {
		if(!(size_c = n->size_constr[i]))
			continue;

		/* Determite the relative element */
		if(size_c->rel == 1) use = &r;
		else use = &p;

		if(size_c->axis == CONSTR_HORI) {
			if(size_c->unit == CONSTR_PCT) {
				value = (int)(use->w * size_c->value);
			}
			else {
				value = (int)size_c->value;
			}

			if(size_c->type == CONSTR_MIN) {
				if(rel->w < value) {
					rel->w = value;
				}
			}
			else if(size_c->type == CONSTR_MAX) {
				if(rel->w > value) {
					rel->w = value;
				}
			}
			else {
				rel->w = value;
			}
		}
		else if(size_c->axis == CONSTR_VERT) {
			if(size_c->unit == CONSTR_PCT) {
				value = (int)(use->h * size_c->value);
			}
			else {
				value = (int)size_c->value;
			}

			if(size_c->type == CONSTR_MIN) {
				if(rel->h < value) {
					rel->h = value;
				}
			}
			else if(size_c->type == CONSTR_MAX) {
				if(rel->h > value) {
					rel->h = value;
				}
			}
			else {
				rel->h = value;
			}
		}
	}

	/* Calculate the absolute position and size of the element */
	abs->x = rel->x - style->bor;
	abs->y = rel->y - style->bor;
	abs->w = rel->w + (style->bor * 2);
	abs->h = rel->h + (style->bor * 2);

	/* Calculate the position and size to be used by child nodes */
	body->x = rel->x + style->ins[3];
	body->y = rel->y + style->ins[0];
	body->w = rel->w - (style->ins[1] + style->ins[3]);
	body->h = rel->h - (style->ins[0] + style->ins[2]);
}

extern void ui_adjust(ui_node *n)
{
	ui_run_down(n, &ui_adjust_node, NULL, 0);
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
