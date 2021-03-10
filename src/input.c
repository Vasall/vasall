#include "input.h"
#include "network.h"

#include <stdlib.h>


/* Redefine the external input-wrapper */
struct input_wrapper input;


extern int inp_init(void)
{
	int i;

	/* Clear the input-pipes */
	inp_pipe_clear(INP_PIPE_IN);
	inp_pipe_clear(INP_PIPE_OUT);

	/* Clear the input-buffers */
	vec2_clr(input.mov);
	vec3_clr(input.dir);
	vec2_clr(input.mov_old);
	vec3_clr(input.dir_old);

	return 0;
}


extern void inp_close(void)
{
	/*  */
}


extern void inp_pipe_clear(enum input_pipe_mode m)
{
	struct input_pipe *pipe;
	int i;

	/* Get a pointer to the associated pipe */
	if(m == INP_PIPE_IN) {
		pipe = &input.pipe_in;
	}
	else if(m == INP_PIPE_OUT) {
		pipe = &input.pipe_out;
	}

	/* Reset number of entries */
	pipe->num = 0;

	for(i = 0; i < INP_ENT_LIM; i++) {
		pipe->type[i] = 0;
	}
}


extern void inp_change(enum input_type type, uint32_t ts, void *in)
{
	ts = ceil(ts / TICK_TIME) * TICK_TIME;

	switch(type) {
		case(INP_T_MOV):
			vec2_cpy(input.mov, (float *)in);

			/* Check if the value really has changed */
			if(!vec2_cmp(input.mov, input.mov_old)) {
				/* If yes, then save value and mark change */
				vec2_cpy(input.mov_old, input.mov);

				/* Push new entries into the in- and out-pipe */
				inp_push(INP_PIPE_IN, objects.id[core.obj],
						INP_T_MOV, ts, input.mov);
				inp_push(INP_PIPE_OUT, objects.id[core.obj],
						INP_T_MOV, ts, input.mov);
			}
			break;

		case(INP_T_DIR):
			vec3_cpy(input.dir, (float *)in);

			/* Check if the value really has changed */
			if(!vec3_cmp(input.dir, input.dir_old)) {
				/* If yes, then save value and mark change */
				vec3_cpy(input.dir_old, input.dir);

				/* Push new entries into the in- and out-pipe */
				inp_push(INP_PIPE_IN, objects.id[core.obj],
						INP_T_DIR, ts, input.dir);
				inp_push(INP_PIPE_OUT, objects.id[core.obj],
						INP_T_DIR, ts, input.dir);
			}
			break;
	}
}


extern int inp_retrieve(enum input_type type, void *out)
{
	switch(type) {
		case(INP_T_MOV):
			vec2_cpy(out, input.mov);
			break;

		case(INP_T_DIR):
			vec3_cpy(out, input.dir);
			break;
	}

	return 0;
}


extern int inp_push(enum input_pipe_mode m, uint32_t id, enum input_type type,
		uint32_t ts, void *in)
{
	char num;	
	struct input_pipe *pipe;

	/* Get a pointer to the associated pipe */
	if(m == INP_PIPE_IN) {
		pipe = &input.pipe_in;
	}
	else if(m == INP_PIPE_OUT) {
		pipe = &input.pipe_out;
	}

	/* If the input-pipe is already full */
	num = pipe->num;
	if(num + 1 >= INP_ENT_LIM)
		return -1;

	pipe->obj_id[num] = id;
	pipe->type[num] = type;
	pipe->ts[num] = ts;

	switch(type) {
		case INP_T_MOV:
			vec2_cpy(pipe->mov[num], (float *)in);
			break;

		case INP_T_DIR:
			vec3_cpy(pipe->dir[num], (float *)in);
			break;
	}

	pipe->num++;
	return 0;
}


extern int inp_pull(struct input_entry *ent)
{
	int i;
	struct input_pipe *pipe = &input.pipe_in;

	if(pipe->num <= 0)
		return 0;

	i = pipe->order[pipe->num - 1];

	ent->id = pipe->obj_id[i];
	ent->type = pipe->type[i];
	ent->ts = pipe->ts[i];
	
	switch(pipe->type[i]) {
		case(INP_T_MOV):
			vec2_cpy(ent->mov, pipe->mov[i]);
			break;

		case(INP_T_DIR):
			vec3_cpy(ent->dir, pipe->dir[i]);
			break;
	}

	/* Reduce number of entries */
	pipe->num -= 1;

	return 1;
}


extern uint32_t inp_next_ts(void)
{
	struct input_pipe *pipe = &input.pipe_in;

	if(pipe->num <= 0)
		return 0;

	return pipe->ts[pipe->order[pipe->num - 1]];
}


extern int inp_pack(char *out)
{
	short i;
	short s;
	short num;
	char *ptr = out;
	enum input_type type;
	int written = 0;
	uint32_t base_ts = 0;
	uint16_t del_ts;
	uint32_t tmp_ts;

	struct input_pipe *pipe = &input.pipe_out; 

	if((num = pipe->num) < 1) {
		return 0;
	}

	tmp_ts = base_ts = pipe->ts[0];

	memcpy(ptr, &base_ts, 4);
	memcpy(ptr + 4, &num, 1);

	ptr += 5;
	written += 5;

	for(i = 0; i < num; i++) {
		s = pipe->order[i];

		memcpy(ptr, &pipe->obj_id[s], 4);
		ptr += 4;
		written += 4;

		memcpy(ptr, &pipe->type[s], 1);
		ptr += 1;
		written += 1;

		del_ts = ((pipe->ts[s] - base_ts) - (tmp_ts - base_ts));
		tmp_ts = pipe->ts[s];

		memcpy(ptr, &del_ts, 2);
		ptr += 2;
		written += 2;

		switch(pipe->type[s]) {
			case INP_T_MOV:
				vec2_cpy((float *)ptr, pipe->mov[s]);
				ptr += VEC2_SIZE;
				written += VEC2_SIZE;
				break;

			case INP_T_DIR:
				vec3_cpy((float *)ptr, pipe->dir[s]);
				ptr += VEC3_SIZE;
				written += VEC3_SIZE;
				break;
		}
	}

	return written;
}


extern int inp_unpack(char *in)
{
	short slot;
	char i;
	char num;

	uint32_t  id;
	uint8_t   mask;
	uint16_t  off;

	vec2_t   mov;
	vec3_t   cam;

	uint16_t act = 0;
	uint32_t ts;

	char *ptr = in;
	int read = 0;

	/* Extract timestamp */
	memcpy(&ts, ptr, 4);

	/* Extract the number of new inputs */
	memcpy(&num,  ptr + 4,  1);

	/* Check if the number of given entries is valid */
	if(num <= 0)
		return -1;

	/* Update pointer-position */
	ptr += 5;
	read += 5;

	for(i = 0; i < num; i++) {
		memcpy(&id, ptr, 4);
		ptr += 4;

		memcpy(&type, ptr, 1);
		ptr += 1;

		memcpy(&off, ptr, 2);
		ptr += 2;

		/* Update timestamp */
		ts += off;

		switch(type) {
			case MOD_T_MOV:
				vec2_cpy(mov, (float *)ptr);
				ptr += VEC2_SIZE;

				if(inp_push(INP_PIPE_IN, id, ts, type, mov) < 0)
					return -1;
				break;

			case MOD_T_CAM:
				vec3_cpy(cam, (float *)ptr);
				ptr += VEC3_SIZE;

				if(inp_push(INP_PIPE_IN, id, ts, type, cam) < 0)
					return -1;
				break;
		}

		read += 13;
	}

	return read;
}


static void inp_pipe_sort(enum input_pipe_mode m)
{
	int i;
	struct input_pipe *pipe;
	char found;
	short tmp;
	short a;
	short b;

	if(m == INP_PIPE_IN)
		pipe = &input.pipe_in;
	else if(m == INP_PIPE_OUT)
		pipe = &input.pipe_out;


	/* Reset order-list */
	for(i = 0; i < INP_ENT_LIM; i++)
		pipe->order[i] = i;


	/*
	 * TODO:
	 * It's 2am, I'm tired and hungry. Bubble-Sort will have to do now, but
	 * please rewrite this to something better that hasn't a runtime
	 * complexity of O(n^2). Thanks <3
	 */

	/* Sort entry-order to decending time. */
	while(1) {
		found = 0;

		for(i = 0; i < pipe->num - 1; i++) {
			a = pipe->order[i];
			b = pipe->order[i + 1];

			if(pipe->ts[a] < pipe->ts[b]) {
				found = 1;

				pipe->order[i] = b;
				pipe->order[i + 1] = a;
			}
		}

		if(!found)
			break;
	}
}

extern void inp_update(uint32_t ts)
{
	/* Sort the entries in both pipes from lowest to hights */
	inp_pipe_sort(INP_PIPE_IN);
	inp_pipe_sort(INP_PIPE_OUT);
}
