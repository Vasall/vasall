#include "controller.h"

#include <stdlib.h>


/* Redefine global controller struct */
struct controller_wrapper controllers;


extern int ctr_init(void)
{
	int i;

	/* Clear the controller-list */
	for(i = 0; i < CTR_DEVICE_LIM; i++) {
		controllers.mask[i] = 0;
	}

	/* Load all available controllers-devices */
	for(i = 0; i < SDL_NumJoysticks(); i++) {
		if(ctr_add_device(i) < 0) {
			goto err_close;
		}
	}

	return 0;

err_close:
	ctr_close();
	return -1;
}


extern void ctr_close(void)
{
	int i;
	int id;
	SDL_GameController *ctr;

	for(i = 0; i < CTR_DEVICE_LIM; i++) {
		if(controllers.mask[i] == 0) {
			continue;
		}

		/* Remove the controller using the ID */
		id = controllers.id[i];
		if((ctr = SDL_GameControllerFromInstanceID(id))) {
			SDL_GameControllerClose(ctr);
		}

		/* Reset mask */
		controllers.mask[i] = 0;
	}
}


extern short ctr_get_id(int id)
{
	short i;

	for(i = 0; i < CTR_DEVICE_LIM; i++) {
		if(controllers.mask[i] == 0)
			continue;

		if(controllers.id[i] == id)
			return i;
	}

	return -1;
}


static short ctr_get_slot(void)
{
	short i;

	for(i = 0; i < CTR_DEVICE_LIM; i++) {
		if(controllers.mask[i] == 0)
			return i;
	}

	return -1;
}


extern int ctr_add_device(int idx)
{
	int32_t id;
	short slot;
	SDL_Joystick *joy;
	SDL_GameController *contr;

	if((slot = ctr_get_slot()) < 0)
		return -1;

	/* Only register if the device is controller */
	if(SDL_IsGameController(idx)) {
		if(!(contr = SDL_GameControllerOpen(idx)))
			return -1;

		joy = SDL_GameControllerGetJoystick(contr);
		id = SDL_JoystickInstanceID(joy);

		controllers.mask[slot] = 1;
		controllers.id[slot] = id;
		controllers.type[slot] = 0;
		strcpy(controllers.name[slot], SDL_GameControllerName(contr));
		controllers.ptr[slot] = contr;	
		return 0;
	}

	return -1;
}


extern void ctr_remv_device(int id)
{
	short slot;

	if((slot = ctr_get_id(id)) < 0)
		return;

	SDL_GameControllerClose(controllers.ptr[slot]);
	controllers.mask[slot] = 0;
}
