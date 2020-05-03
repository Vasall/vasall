#ifndef _CORE_H
#define _CORE_H

#include "defines.h"
#include "vec.h"
#include "mat.h"
#include "list.h"
#include "hashtable.h"
#include "window.h"
#include "render.h"
#include "camera.h"
#include "model.h"
#include "model_utils.h"
#include "object.h"
#include "world.h"

struct core_wrapper {
	char *bindir;
	int8_t running;
	void (*procevt)(XSDL_Event *e);
	void (*update)(void);
	void (*render)(void);

	short obj;
};

extern uint8_t one;
extern uint8_t zero;
extern int g_win_flgs;
extern struct core_wrapper *core;
extern char glo_err_buf[256];


extern void glo_set_err(char *err);
extern char *glo_get_err(void);


extern int core_init(int argc, char **argv);
extern void core_close(void);

extern int core_load(char *pth);

extern void core_update(void);
extern void core_render(void);

#endif
