#ifndef _NODE_H
#define _NODE_H

#include "sdl.h"

typedef struct ui_node ui_node;

typedef void (*ui_fnc)(ui_node *n, void *d);
typedef void (*ui_node_fnc)(ui_node *n, event_t *e);
typedef void (*ui_render_fnc)(ui_node *n, ui_node *rel);

/*
 * The different tags of the different nodes.
 */
typedef enum ui_tag {
	UI_WRAPPER,    /* Wrapping together a bunch of lower nodes */
	UI_TEXT,       /* Display text */
	UI_BUTTON,     /* Clickable button  */
	UI_INPUT       /* Input field for text and numbers */
} ui_tag;

typedef enum ui_cst_mask {
	UI_CST_NONE   = -1,
	UI_CST_AUTO   = 0,
	UI_CST_LEFT   = 1,
	UI_CST_RIGHT  = 2,
	UI_CST_TOP    = 3,
	UI_CST_BOTTOM = 4
} ui_cst_mask;

typedef enum ui_cst_unit {
	UI_CST_PX,
	UI_CST_PCT
} ui_cst_unit;

typedef enum ui_cst_rel {
	UI_CST_REL = 0,     /* Calculate size relative to parent-node-size */
	UI_CST_ABS = 1,     /* Calculate size relative to surface-size */
	UI_CST_WIN = 2      /* Calculate size relative to window-size */
} ui_cst_rel;

typedef enum ui_cst_type {
	UI_CST_POS,
	UI_CST_SIZE
} ui_cst_type;

typedef enum ui_cst_algn {
	UI_CST_HORI = 0,
	UI_CST_VERT = 1
} ui_cst_algn;

typedef enum ui_cst_mod {
	UI_CST_SET  = 0,    /* Set the value of the constraint */
	UI_CST_LLIM = 1,    /* Use value to limit downwards */
	UI_CST_HLIM = 2     /* Use value to limit upwards */
} ui_cst_mod;

/*
 * A single constrain entry.
 *
 * mask: The mask indicates what the aligment and type of the constraint is
 * value: If absolute use absolute pixel value, 0.0-1.0 for percentage
 * unit: Indicated how the value should be processed
 * rel: What to calculate the size relative to if percentage
 */
typedef struct ui_cst_ent {
	ui_cst_mask  mask; 
	float           value;
	ui_cst_unit  unit;
	ui_cst_rel   rel;
} ui_cst_ent;

/*
 * This containt unit is used to set and limit the size of a node.
 *
 * Row 1 is horizontal
 * Row 2 is vertical
 *
 * Slot 0 is used to set the values
 * Slot 1 is used to limit lower numbers
 * Slot 2 is used to limit higher numbers
 */
typedef struct ui_cst_wrp {
	ui_cst_ent ent[2][3];
} ui_cst_wrp;

/* Default position- and size-constraints */
extern const ui_cst_wrp   UI_POS_CST_NULL;
extern const ui_cst_wrp   UI_POS_CST_NULL;

typedef struct ui_node_flags {
	uint8_t active;
	uint8_t procevt;
	uint8_t enfoc;
	uint8_t cur;
} ui_node_flags;

extern const ui_node_flags   UI_NULL_FLAGS;

#define FLG_ACT		0x00
#define FLG_EVT		0x01
#define FLG_FOC		0x02
#define FLG_CUR		0x03

typedef struct ui_node_style {
	uint8_t      vis;
	uint8_t      bck;
	color_t      bck_col;
	short        bor;
	color_t      bor_col;
	short        cor_rad[4];
} ui_node_style;

extern const ui_node_style   UI_NULL_STYLE;

#define UI_STY_VIS	0x00
#define UI_STY_BCK	0x01
#define	UI_STY_BCKCOL	0x02
#define UI_STY_BOR	0x03
#define UI_STY_BORCOL	0x04
#define UI_STY_CRNRAD	0x05

typedef struct ui_node_events {
	ui_node_fnc focus;
	ui_node_fnc unfocus;
	ui_node_fnc onactive;
	ui_node_fnc mousedown;
	ui_node_fnc mouseup;
	ui_node_fnc hover;
	ui_node_fnc keydown;
	ui_node_fnc keyup;
	ui_node_fnc textinput;
} ui_node_events;

extern const ui_node_events  UI_NULL_EVENTS;

#define EVT_MOUSEDOWN	0x00
#define EVT_MOUSEUP	0x01
#define EVT_HOVER	0x02
#define EVT_KEYDOWN	0x03
#define EVT_KEYUP	0x04

#define EVT_WIN_RESIZE	0x20

#define UI_NAME_LEN     20
#define UI_CHILD_NUM    8

struct ui_node {
	char    id[UI_NAME_LEN];
	short   layer;
	ui_tag  tag;

	ui_node *parent;

	short    child_num;
	ui_node  *children[UI_CHILD_NUM];

	void *element;

	ui_render_fnc  render;
	ui_fnc         del;

	ui_node_flags   flags;
	ui_node_style   style;
	ui_node_events  events;

	ui_cst_wrp pos_constr;
	ui_cst_wrp size_constr;

	/* The position and size of the node relative to the window */
	rect_t body;

	/* The position and size of the node relative to the parent node */
	rect_t rel_body;

	uint32_t vao;
	uint32_t bao[2];
	uint32_t tex;

	surf_t *surf;
};

extern const float  STD_CORNERS[18];
extern const float  STD_UV[12];


/*
 * Add a new node to the node-tree and attach an element to it if necessary.
 * Note that the node will be initialized depending on the given tag and will
 * use the default position- and size-constraints.
 *
 * @tag: The tag of the node
 * @par: The parent node to attach the new node to
 * @ele: Optional pointer to the element to attach to the node
 * @id: The string-id of the node
 *  
 * Returns: Pointer to the new node
 */
extern ui_node *ui_add(ui_tag tag, ui_node *par, void *ele, char *id);


/*
 * Remove a node and all it's child-nodes from the node-tree.
 *
 * @n: Pointer to the node to remove
 */
extern void ui_remv(ui_node *n);


/*
 * Get a node from the node-tree by searching for a certain node-id. Note that
 * this function will always return the first result.
 *
 * @n: The node to start from searching
 * @id: The id of the node to search for
 *
 * Returns: Either a pointer to the node or NULL if an error occurred
 */
extern ui_node *ui_get(ui_node *n, char *id);


#define UI_DOWN_POST (1<<0)
#define UI_DOWN_ALL  (1<<1)

/*
 * Recursivly go down layer by layer and apply the given function to the nodes.
 * Note that this function will use preorder to traverse the tree by default.
 * Use flag UI_DOWN_POST to enable postorder traversal.
 *
 * @n: The node to start from
 * @fnc: The function to apply to the nodes
 * @data: Optional data to share between the nodes
 * @flg: Flags to consider when going through the layers
 */
extern void ui_down(ui_node *n, ui_fnc fnc, void *data, uint8_t flg);


/*
 * Recursivly go up layer by layer from a given node to the root node and apply
 * a function to all nodes.
 *
 * @n: The node to start from
 * @fnc: The function to apply to the nodes
 * @data: Optional data to share between the nodes
 */
extern void ui_up(ui_node *n, ui_fnc fnc, void *data);


/*
 * Add a new constraint to a node.
 *
 * @n: Pointer to the node
 * @type: Indicates if the constraint should be used to set the position or the
 * 	  size of the node (use either UI_CST_POS or UI_CST_SIZE)
 * @algn: Indicated if the constraint shoule be used on the horizontal or
 * 	  vertical axis (use either UI_CST_HORI or UI_CST_VERT)
 * @mod: Define the kind of constraint and how the calculated value shoule be
 *       used (0: set value, 1: limit downwards, 2: limit upwards)
 * @val: If UI_CST_PX use absolute pixel-value, if UI_CST_PCT use range 0.0-1.0
 * @unit: What unit should the given value be assigned (UI_CST_PX or UI_CST_PCT)
 * @rel: If the unit is percent, relative to what should the value be when
 *       calculating
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int ui_constr(ui_node *n, ui_cst_type type, ui_cst_algn algn,
		ui_cst_mod mod, ui_cst_mask mask, float val,
		ui_cst_unit unit, ui_cst_rel rel);


/*
 * Prerender a certain node onto a surface of a render-node with can be either
 * the node itself or a node a few layers above. The function will go up layer
 * by layer to find a node with an attached surface and texture, clear the
 * surface and then prerender the all child nodes onto the surface. Finally it
 * will update the texture.
 *
 * @n: The node to prerender
 */
extern void ui_update(ui_node *n);


/*
 * Render a node and all of it's active children on the screen. Note that the
 * given node is required to have a surface and texture attached to it.
 *
 * @n: The node to render on the screen
 */
extern void ui_render(ui_node *n);


/*
 * Start from the given node and go down layer by layer and adjust the position
 * and size of the given nodes.
 *
 * @n: The node to start from
 */
extern void ui_adjust(ui_node *n);


/*
 * Attach a surface and a texture to a node. This enables the node itself and 
 * it's child nodes to be prerendered.
 *
 * @n: Pointer to the node to attach a surface and texture to
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int ui_enable_tex(ui_node *n);


/*
 * Adjust the size of the surface and texture to the size of the node. Note 
 * that the node is required to have a surface and texture attached to it.
 *
 * @n: Pointer to the node
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int ui_update_node(ui_node *n);


/*
 * Modify a flag of a node and update the value.
 *
 * @n: Pointer to the node
 * @flg: The flag to modify
 * @val: A buffer containing the new value
 */
extern void ui_set_flag(ui_node *n, short flg, void *val);


/*
 * Modify a style-attribute of a node and update the value.
 *
 * @n: Pointer to the node
 * @attr: The attribute to modify
 * @val: A buffer containing the new value
 */
extern void ui_style(ui_node *n, short attr, void *val);


/*
 * Attach callback-function for a certain event to a node.
 *
 * @n: Pointer to the node
 * @evt: The event to attach the function to
 * @fnc: The function to attach to the node
 */
extern void ui_event(ui_node *n, short evt, ui_node_fnc fnc);


#define UI_SHOW_ALL     0
#define UI_SHOW_SINGLE  1

/*
 * Display the node-tree starting from a certain node in the console. This
 * function will only be used for debugging.
 *
 * @n: The node to start from
 * @flg: Optional flags 
 */
extern void ui_show(ui_node *n, uint8_t flg);

#endif
