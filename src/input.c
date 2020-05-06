#include "input.h"

#include <stdlib.h>


struct input_wrapper input;


extern int inp_init(void)
{
	int i;

	for(i = 0; i < DEVICE_NUM; i++)
		input.mask[i] = 0;

	for(i = 0; i < SDL_NumJoysticks(); i++) {
		if(inp_add_device(i) < 0)
			goto err_close_input;
	}

	return 0;

err_close_input:
	inp_close();
	return -1;
}

extern void inp_close(void)
{
	int i;
	SDL_GameController *contr;

	for(i = 0; i < DEVICE_NUM; i++) {
		if(input.mask[i] == 0)
			continue;

		if((contr = SDL_GameControllerFromInstanceID(input.id[i])))
			SDL_GameControllerClose(contr);
	}
}

static short inp_get_slot(void)
{
	short i;

	for(i = 0; i < DEVICE_NUM; i++) {
		if(input.mask[i] == 0)
			return i;
	}

	return -1;
}

extern int inp_add_device(int idx)
{
	int32_t id;
	short slot;
	SDL_Joystick *joy;
	SDL_GameController *contr;

	if((slot = inp_get_slot()) < 0)
		return -1;

	/* Only register if the device is controller */
	if(SDL_IsGameController(idx)) {
		if(!(contr = SDL_GameControllerOpen(idx)))
			return -1;

		joy = SDL_GameControllerGetJoystick(contr);
		id = SDL_JoystickInstanceID(joy);

		input.mask[slot] = 1;
		input.id[slot] = id;
		input.type[slot] = 0;
		strcpy(input.name[slot], SDL_GameControllerName(contr));
		input.ptr[slot] = contr;	
		return 0;
	}

	return -1;
}

static short inp_get_id(int id)
{
	short i;

	for(i = 0; i < DEVICE_NUM; i++) {
		if(input.mask[i] == 0)
			continue;

		if(input.id[i] == id)
			return i;
	}

	return -1;
}

extern void inp_remv_device(int id)
{
	short slot;

	if((slot = inp_get_id(id)) < 0)
		return;

	SDL_GameControllerClose(input.ptr[slot]);
	input.mask[slot] = 0;
}
