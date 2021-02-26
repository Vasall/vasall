#ifndef _INPUT_H
#define _INPUT_H

#include "sdl.h"
#include "vector.h"

#define INPUT_SLOTS    16

#define INP_M_MOV      (1<<0)
#define INP_M_ACT      (1<<1)

struct input_buffer {
	short       num;
	uint32_t    obj;

	uint32_t    mask[INPUT_SLOTS];
	uint32_t    ts[INPUT_SLOTS];

	vec2_t      mov[INPUT_SLOTS];
	uint32_t    act[INPUT_SLOTS];
};


#define DEVICE_NUM     4

struct input_wrapper {
	/* The device-table */
	char       mask[DEVICE_NUM];
	int        id[DEVICE_NUM];
	uint8_t    type[DEVICE_NUM];
	char       name[DEVICE_NUM][24];
	void       *ptr[DEVICE_NUM];

	/* The current input-buffer with the different input-values */
	vec2_t mov;
	vec3_t cam;

	/* The share-buffer to share with peers */
	struct input_buffer share;

	vec2_t mov_old;
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


/*
 * Collect all entries from the share-buffer and write them in the default
 * share-format to the buffer.
 *
 * @buf: The buffer to write the data to (has to be allocated already)
 *
 * Returns: The number of bytes written
 */
extern int inp_col_share(char *buf);

#endif
