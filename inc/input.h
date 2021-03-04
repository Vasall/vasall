#ifndef _INPUT_H
#define _INPUT_H

#include "sdl.h"
#include "vector.h"

#define INP_ENT_LIM   16

#define INP_CHG_MOV (1<<0)
#define INP_CHG_DIR (1<<1)


enum input_pipe_mode {
	INP_PIPE_IN,
	INP_PIPE_OUT
};

enum input_type {
	INP_T_NONE = 0,
	INP_T_MOV =  1,
	INP_T_DIR =  2
};

struct input_entry {
	uint32_t  id;
	uint8_t   type;
	uint32_t  ts;

	vec2_t    mov;
	vec3_t    dir;
};


struct input_pipe {
	char        num;
	short       order[INP_ENT_LIM];

	uint32_t    obj_id[INP_ENT_LIM];
	uint8_t     type[INP_ENT_LIM];
	uint32_t    ts[INP_ENT_LIM];

	vec2_t      mov[INP_ENT_LIM];
	vec3_t      dir[INP_ENT_LIM];
};

struct input_wrapper {
	/* The current input-buffer with the different input-values */
	vec2_t mov;
	vec3_t dir;

	vec2_t mov_old;
	vec3_t dir_old;

	/* The share-buffer to share with peers */
	struct input_pipe pipe_in;
	struct input_pipe pipe_out;
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
 * Clear the specified pipe and reset the number of entries.
 *
 * @m: The pipe to clear
 */
extern void inp_pipe_clear(enum input_pipe_mode m);

extern void inp_change(enum input_type type, uint32_t ts, void *in);

extern int inp_retrieve(enum input_type type, void *out);

/*
 * Push a new entry to the specified pipe.
 *
 * @m: The pipe to push the entry to
 * @id: The ID of the affiliated object
 * @type: The type of input
 * @ts: The timestamp at which the input occurred in network time(milliseconds)
 * @in: A buffer with the input-data
 * 
 * Returns: Either 0 on success or -1 if an error occurred
 */
extern int inp_push(enum input_pipe_mode m, uint32_t id, enum input_type type,
		uint32_t ts, void *in);


/*
 * Pull an entry from the in-pipe and write the data to the given pointer.
 * Note that this function will remove the entry from the list and therefore
 * reduce the number of entries.
 *
 * @ent: A pointer to write the entry to
 *
 * Returns: 1 if an entry has been returned, or 0 if the list is empty
 */
extern int inp_pull(struct input_entry *ent);


/*
 * Collect all entries in the out-pipe and write them in the default
 * input-share-format to the given pointer. Note that the necessary memory
 * already has to be allocated (estimate INP_ENT_LIM * 19 + 5) and the function
 * should only be used after inp_update() has already been called as the entries
 * should be sorted by time.
 *
 * @out: A pointer to write the data to
 *
 * Returns: Either the number of bytes written, 0 if the pipe is empty and -1 if
 *          an error occurred
 */
extern int inp_pack(char *out);


/*
 * Unpack the shared entries, which have to be encoded in the default
 * input-share-format, and push them into the in-pipe.
 *
 * @in: A buffer to read the data from
 *
 * Returns: Either the number of bytes read, or -1 if an error occurred
 */
extern int inp_unpack(char *in);


/*
 * Sort the entries in the pipes and process them. 
 */
extern void inp_update(void);

#endif
