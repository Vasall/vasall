#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "sdl.h"

#include <stdint.h>

#define CTR_DEVICE_LIM 4

struct ctr_wrapper {
	uint32_t   mask[CTR_DEVICE_LIM];
	int        id[CTR_DEVICE_LIM];
	uint8_t    type[CTR_DEVICE_LIM];
	char       name[CTR_DEVICE_LIM][24];
	void       *ptr[CTR_DEVICE_LIM];
};


extern struct ctr_wrapper g_ctr;


extern int ctr_init(void);


extern void ctr_close(void);


extern short ctr_get_id(int id);


extern int ctr_add_device(int idx);


extern void ctr_remv_device(int id);

#endif
