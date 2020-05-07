#ifndef _NODE_H
#define _NODE_H

#include "sdl.h"

typedef struct ui_node ui_node;

typedef void (*ui_node_callback)(ui_node *n, SDL_Event *e);
typedef void (*ui_render_callback)(ui_node *n, ui_node *rel);

typedef enum ui_node_tag {
	UI_WRAPPER,
	UI_TEXT,
	UI_BUTTON,
	UI_INPUT,
	UI_LABEL
} ui_node_tag;

typedef enum ui_constr_type {
	CONSTR_SET,
	CONSTR_MIN,
	CONSTR_MAX
} ui_constr_type;

typedef enum ui_constr_align {
	CONSTR_CENTER,
	CONSTR_LEFT,
	CONSTR_RIGHT,
	CONSTR_TOP,
	CONSTR_BOTTOM
} ui_constr_align;

typedef enum ui_constr_axis {
	CONSTR_VERT,
	CONSTR_HORI
} ui_constr_axis;

typedef enum ui_constr_unit {
	CONSTR_PX,
	CONSTR_PCT
} ui_constr_unit;

typedef struct ui_pos_constr {
	char id[4];
	ui_constr_type type;
	ui_constr_axis axis;
	uint8_t rel;
	ui_constr_align align;
	float value;
	ui_constr_unit unit;
} ui_pos_constr;

typedef struct ui_size_constr {
	char id[4];
	ui_constr_type type;
	ui_constr_axis axis;
	uint8_t rel;
	float value;
	ui_constr_unit unit;
} ui_size_constr;

typedef struct ui_node_flags {
	uint8_t active;
	uint8_t procevt;
	uint8_t enfoc;
	uint8_t cur;
} ui_node_flags;

#define FLG_ACT		0x00
#define FLG_EVT		0x01
#define FLG_FOC		0x02
#define FLG_CUR		0x03

typedef struct ui_node_style {
	uint8_t vis;
	uint8_t bck;
	SDL_Color bck_col;
	short bor;
	SDL_Color bor_col;
	short cor_rad[4];
	short ins[4];
	uint8_t hori_algn;
	uint8_t vert_algn;
} ui_node_style;

#define STY_VIS		0x00
#define STY_BCK		0x01
#define	STY_BCK_COL	0x02
#define STY_BOR		0x03
#define STY_BOR_COL	0x04
#define STY_COR_RAD	0x05
#define STY_INS		0x06
#define STY_HALGN	0x07
#define STY_VALGN	0x08

typedef struct ui_node_events {
	ui_node_callback focus;
	ui_node_callback unfocus;
	ui_node_callback onactive;
	ui_node_callback mousedown;
	ui_node_callback mouseup;
	ui_node_callback hover;
	ui_node_callback keydown;
	ui_node_callback keyup;
	ui_node_callback textinput;
} ui_node_events;

#define EVT_MOUSEDOWN	0x00
#define EVT_MOUSEUP	0x01
#define EVT_HOVER	0x02
#define EVT_KEYDOWN	0x03
#define EVT_KEYUP	0x04

#define EVT_WIN_RESIZE	0x20

#define CHILD_NUM 8

struct ui_node {
	char strid[20];
	short layer;
	ui_node_tag tag;

	ui_node *parent;

	short child_num;
	struct ui_node *children[CHILD_NUM];

	void *element;

	ui_render_callback render;

	void (*del)(ui_node *n, void *data);

	ui_node_flags flags;
	ui_node_style style;
	ui_node_events events;

	ui_pos_constr *pos_constr[6];
	ui_size_constr *size_constr[6];

	SDL_Rect rel;
	SDL_Rect body;
	SDL_Rect abs;

	uint32_t vao;
	uint32_t bao[2];
	uint32_t tex;

	SDL_Surface *surf;
};

extern const ui_node_flags NULL_FLAGS;
extern const ui_node_style NULL_STYLE;
extern const ui_node_events NULL_EVENTS;

const float STD_CORNERS[18];
extern const float STD_UV[12];

extern struct ui_node *ui_add(struct ui_node *par, void *ele, 
		enum ui_node_tag tag, char *strid, SDL_Rect *body);

extern void ui_remv(struct ui_node *n);

extern struct ui_node *ui_get(char *strid);

extern int ui_set_pos_constr(struct ui_node *n, char *id, ui_constr_type type,
		ui_constr_axis axis, uint8_t rel, ui_constr_align align,
		float val, ui_constr_unit unit);

extern void ui_del_pos_constr(struct ui_node *n, char *id);

extern int ui_set_size_constr(struct ui_node *n, char *id, ui_constr_type type,
		ui_constr_axis axis, uint8_t rel, float val, 
		ui_constr_unit unit);

extern void ui_del_size_constr(struct ui_node *n, char *id);

extern void ui_prerender(struct ui_node *n);

extern void ui_render(struct ui_node *n);

#define RUN_DOWN_AFT 1
#define RUN_DOWN_ALL 2

extern void ui_run_down(struct ui_node *n, void (*func)(ui_node *n, void *d),
		void *data, uint8_t flg);

extern void ui_run_up(struct ui_node *n, void (*func)(ui_node *n, void *d), 
		void *data);

extern void ui_show_node(ui_node *node, void *data);
extern void ui_show(struct ui_node *n);

extern void ui_adjust(struct ui_node *n);

extern int ui_enable_tex(struct ui_node *n);

extern void ui_update_tex(struct ui_node *n);

extern void ui_update_surf(struct ui_node *n);

extern void ui_mod_flag(struct ui_node *n, short flag, void *val);

extern void ui_mod_style(ui_node *node, short opt, void *val);

extern void ui_bind_event(struct ui_node *n, short evt, ui_node_callback ptr);

#endif
