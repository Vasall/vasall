#ifndef _INPUT_H_
#define _INPUT_H_

#include "vec.h"
#include "global.h"

extern struct input_binds {
	/*
	 * The current movement-direction.
	 */
	Vec2 mov_dir;
};


/* Define the global input-handler */
extern struct input_binds *input;


/* Initialize the input-handler */
int inpInit(void);

/* Update the current inputs */
void inpUpdate(void);

#endif