#ifndef _WINDOW_H
#define _WINDOW_H

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include "XSDL/XSDL.h"
#include "XSDL/XSDL_cache.h"
#include "XSDL/XSDL_context.h"
#include "XSDL/XSDL_img.h"
#include "XSDL/XSDL_init.h"
#include "XSDL/XSDL_input.h"
#include "XSDL/XSDL_list.h"
#include "XSDL/XSDL_node.h"
#include "XSDL/XSDL_render.h"
#include "XSDL/XSDL_stdnodes.h"
#include "XSDL/XSDL_struct.h"
#include "XSDL/XSDL_text.h"
#include "XSDL/XSDL_utf8.h"
#include "XSDL/XSDL_utils.h"

struct window_wrapper {
	XSDL_Window *win;
	XSDL_GLContext gl_ctx;
	XSDL_UIContext *ui_ctx;
	XSDL_Node *ui_root;
	int8_t fullscr;
};


extern struct window_wrapper *window;


extern int win_init(void);
extern void win_close(void);

#endif
