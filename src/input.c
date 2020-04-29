#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Redefine external-variables */
struct input_map *inp_map = NULL;

int inp_init(void)
{
	if(!(inp_map = calloc(1, sizeof(struct input_map))))
		return -1;

	return 0;
}

void inp_close(void)
{
	if(!inp_map)
		return;

	free(inp_map);
}
