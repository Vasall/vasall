#ifndef _INPUT_H_
#define _INPUT_H_

#include "vec.h"
#include "global.h"

struct inp_device {
	uint8_t type;

	int id;

	void *ptr;
};

struct inp_binds {
	/*
	 * The current movement-direction.
	 */
	Vec2 mov_dir;
};

struct input_map {
	float move[2];
	float camera[2];
};


/* Define the global input-handler */
extern struct inp_binds *input;

extern struct dyn_stack *inp_devices;

extern struct input_map *inp_map;

/* Initialize the input-handler */
int inpInit(void);

/* Close the input-handler */
void inpClose(void);

/* Load available input-devices */
int inpLoadDevices(void);

/* Update the current inputs */
void inpUpdate(void);

#endif
