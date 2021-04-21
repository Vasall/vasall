#include "controller.h"

#include <stdlib.h>


/* Redefine global controller-wrapper */
struct ctr_wrapper g_ctr;


extern int ctr_init(void)
{
	int i;

	/* Clear the controller-list */
	for(i = 0; i < CTR_DEVICE_LIM; i++) {
		g_ctr.mask[i] = 0;
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
		if(g_ctr.mask[i] == 0) {
			continue;
		}

		/* Remove the controller using the ID */
		id = g_ctr.id[i];
		if((ctr = SDL_GameControllerFromInstanceID(id))) {
			SDL_GameControllerClose(ctr);
		}

		/* Reset mask */
		g_ctr.mask[i] = 0;
	}
}


extern short ctr_get_id(int id)
{
	short i;

	for(i = 0; i < CTR_DEVICE_LIM; i++) {
		if(g_ctr.mask[i] == 0)
			continue;

		if(g_ctr.id[i] == id)
			return i;
	}

	return -1;
}


static short ctr_get_slot(void)
{
	short i;

	for(i = 0; i < CTR_DEVICE_LIM; i++) {
		if(g_ctr.mask[i] == 0)
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

		g_ctr.mask[slot] = 1;
		g_ctr.id[slot] = id;
		g_ctr.type[slot] = 0;
		strcpy(g_ctr.name[slot], SDL_GameControllerName(contr));
		g_ctr.ptr[slot] = contr;	
		return 0;
	}

	return -1;
}


extern void ctr_remv_device(int id)
{
	short slot;

	if((slot = ctr_get_id(id)) < 0)
		return;

	SDL_GameControllerClose(g_ctr.ptr[slot]);
	g_ctr.mask[slot] = 0;
}
