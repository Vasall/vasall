#ifndef _INPUT_H
#define _INPUT_H

#include "sdl.h"
#include "vec.h"

#define SHARE_SLOTS 16

#define SHARE_M_MOV    0x01


struct share_buffer {
	short     num;
	uint32_t  timer;

	uint32_t    mask[SHARE_SLOTS];
	uint8_t     off[SHARE_SLOTS];
	uint32_t    obj[SHARE_SLOTS];
	vec2_t      mov[SHARE_SLOTS];
};


#define DEVICE_NUM 4

struct input_wrapper {
	/* The device-table */
	char       mask[DEVICE_NUM];
	int        id[DEVICE_NUM];
	uint8_t    type[DEVICE_NUM];
	char       name[DEVICE_NUM][24];
	void       *ptr[DEVICE_NUM];

	/* The current input-buffer with the different input-values */
	vec2_t mov;
	vec2_t cam;

	/* The share-buffer to share with peers */
	struct share_buffer share;

	vec3_t mov_old;
};


/* The global input-wrapper */
extern struct input_wrapper input;


/*
 * Initialize the global input-wrapper and load all available input-devices.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int inp_init(void);


/*
 * Close the global input-wrapper, remove all active input-devices and free
 * the allocated memory.
 */
extern void inp_close(void);


/*
 * Add a new input-devices to the device-table.
 *
 * @idx: The index of the new input-device
 *
 * Returns: Either 0 on success or -1 if an error occurred
 */
extern int inp_add_device(int idx);


/*
 * Remove an active device from the device-table and free the allocated memory.
 *
 * @id: The id of the device to remove
 */
extern void inp_remv_device(int id);

#endif
