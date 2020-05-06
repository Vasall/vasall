#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "filesystem.h"
#include "sdl.h"

#include <stdint.h>

#define TEX_NAME_MAX            8
#define TEX_SLOTS               8

struct texture_wrapper {
	uint8_t mask[TEX_SLOTS];
	char name[TEX_SLOTS][9];
	uint32_t handle[TEX_SLOTS];
};


extern struct texture_wrapper textures;


extern int tex_init(void);
extern void tex_close(void);

extern short tex_set(char *name, uint8_t *px, int w, int h);
extern void tex_del(short slot);
extern short tex_get(char *name);

extern void tex_use(short slot);
extern void tex_unuse(void);

extern short tex_load_png(char *name, char *pth);

#endif
