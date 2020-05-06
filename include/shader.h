#ifndef _SHADER_H
#define _SHADER_H

#include "sdl.h"

#include <stdint.h>

#define SHD_NAME_MAX            8
#define SHD_SLOTS               8

struct shader_wrapper {
	uint8_t mask[SHD_SLOTS];
	char name[SHD_SLOTS][9];
	uint32_t prog[SHD_SLOTS];
};


extern struct shader_wrapper shaders;


extern int shd_init(void);
extern void shd_close(void);

extern short shd_set(char *name, char *vs, char *fs);
extern void shd_del(short slot);

extern short shd_get(char *name);

extern void shd_use(short slot, int *loc);
extern void shd_unuse(void);



#endif
