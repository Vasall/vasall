#ifndef _INPUT_H
#define _INPUT_H

#include "sdl.h"
#include "vec.h"

#define DEVICE_NUM 4

struct input_wrapper {
	char mask[DEVICE_NUM];
	int id[DEVICE_NUM];
	uint8_t type[DEVICE_NUM];
	char name[DEVICE_NUM][24];
	void *ptr[DEVICE_NUM];

	vec2_t movement;
	vec2_t camera;
};


extern struct input_wrapper input;


extern int inp_init(void);
extern void inp_close(void);

extern int inp_add_device(int idx);
extern void inp_remv_device(int id);

#endif
