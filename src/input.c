#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include "global.h"

/* Redefine external variables */
struct inp_binds *input = NULL;

struct dyn_stack *inp_devices = NULL; 

struct input_map *inp_map = NULL;

/*
 * Initialize the input-binds and allocate
 * the necessary memory.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int inpInit(void)
{
	input = malloc(sizeof(struct inp_binds));
	if(input == NULL) goto failed;

	inp_devices = stcCreate(sizeof(struct inp_device));
	if(inp_devices == NULL) goto failed;

	inp_map = malloc(sizeof(struct input_map));
	if(inp_map == NULL) goto failed;
	memset(inp_map, 0, sizeof(struct input_map));

	return(0);

failed:
	free(input);
	stcDestroy(inp_devices);
	free(inp_map);

	return(-1);	
}

/*
 * Close the input-binds and free the 
 * allocated memory.
 */
void inpClose(void)
{
	free(input);
	stcDestroy(inp_devices);
	free(inp_map);
}

/*
 * Load all available devices connected to the
 * system and push them into the device-stack.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int inpLoadDevices(void)
{
	int i;
	SDL_GameController *contr = NULL;
	SDL_Joystick *joy = NULL;

	for(i = 0; i < SDL_NumJoysticks(); i++) {
		if(SDL_IsGameController(i)) {
			contr = SDL_GameControllerOpen(i);
			if(contr != NULL) {
				printf("Init device '%s'\n",
						SDL_GameControllerName(contr));
			}
			else {
				printf("Could not open controller %d\n", i);
			}
		}
	}

	return(0);
}
