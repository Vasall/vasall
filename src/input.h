#ifndef _INPUT_H_
#define _INPUT_H_

struct input_map {
	float camera[2];

	float movement[2];
};

extern struct input_map *inp_map;

/* Initialize the input-map */
int inpInit(void);

#endif
