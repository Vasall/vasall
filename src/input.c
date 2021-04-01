#include "input.h"
#include "network.h"
#include "core.h"
#include "object.h"

#include <stdlib.h>


/* Redefine the external input-wrapper */
struct input_wrapper input;


extern int inp_init(void)
{
	input.mask = 0;

	/* Clear the input-pipes */
	inp_pipe_clear(INP_PIPE_IN);
	inp_pipe_clear(INP_PIPE_OUT);

	/* Clear the input-buffers */
	vec2_clr(input.mov);
	vec3_clr(input.dir);
	vec2_clr(input.mov_old);
	vec3_clr(input.dir_old);

	input.log.start = 0;

	/* Reset the log */
	inp_reset();

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
	switch(type) {
		case INP_T_NONE:
			break;

		case INP_T_MOV:
			input.mask |= 1;
			input.mov_ts = ts; 
			vec2_cpy(input.mov, (float *)in);
			break;

		case INP_T_DIR:
			input.mask |= 2;
			input.dir_ts = ts;
			vec3_cpy(input.dir, (float *)in);
			break;
	}
}


extern int inp_retrieve(enum input_type type, void *out)
{
	switch(type) {
		case INP_T_NONE:
			break;

		case INP_T_MOV:
			vec2_cpy(out, input.mov);
			break;

		case INP_T_DIR:
			vec3_cpy(out, input.dir);
			break;
	}

	return 0;
}


extern int inp_push(enum input_pipe_mode m, uint32_t id, enum input_type type,
		uint32_t ts, void *in)
{
	short i;
	short num;	
	struct input_pipe *pipe;

	/* Get a pointer to the associated pipe */
	if(m == INP_PIPE_IN) {
		pipe = &input.pipe_in;
	}
	else if(m == INP_PIPE_OUT) {
		pipe = &input.pipe_out;
	}

	/* 
	 * Check if an entry for an object with the same timestamp and type is
	 * already in list and overwrite it.
	 */
	for(i = 0; i < pipe->num; i++) {
		if(pipe->ts[i] == ts && pipe->obj_id[i] == id) {
			if(pipe->type[i] == type) {
				switch(type) {
					case INP_T_NONE:
						break;

					case INP_T_MOV:
						vec2_cpy(pipe->mov[i], (float *)in);
						break;

					case INP_T_DIR:
						vec3_cpy(pipe->dir[i], (float *)in);
						break;
				}

				return 0;
			}
		}
	}


	/* If the input-pipe is already full */
	num = pipe->num;
	if(num + 1 >= INP_ENT_LIM)
		return -1;

	pipe->obj_id[num] = id;
	pipe->type[num] = type;
	pipe->ts[num] = ts;

	switch(type) {
		case INP_T_NONE:
			break;

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

	if(pipe->num < 1)
		return 0;

	i = pipe->order[pipe->num - 1];

	ent->obj_id = pipe->obj_id[i];
	ent->type = pipe->type[i];
	ent->ts = pipe->ts[i];

	switch(pipe->type[i]) {
		case INP_T_MOV:
			vec2_cpy(ent->mov, pipe->mov[i]);
			break;

		case INP_T_DIR:
			vec3_cpy(ent->dir, pipe->dir[i]);
			break;
	}

	/* Reduce number of entries */
	pipe->num -= 1;

	return 1;
}


extern void inp_pipe_print(enum input_pipe_mode m)
{
	int i;
	struct input_pipe *pipe;

	printf("-\n");

	/* Get a pointer to the associated pipe */
	if(m == INP_PIPE_IN) {
		pipe = &input.pipe_in;
	}
	else if(m == INP_PIPE_OUT) {
		pipe = &input.pipe_out;
	}

	for(i = 0; i < pipe->num; i++) {
		printf("%d: %8x >> type: %2d - ts: %6x - ", i, pipe->obj_id[i],
				pipe->type[i], pipe->ts[i]);

		switch(pipe->type[i]) {
			case INP_T_NONE:
				break;

			case INP_T_MOV:
				vec2_print(pipe->mov[i]);
				break;

			case INP_T_DIR:
				vec3_print(pipe->dir[i]);
				break;
		}

		printf("\n");
	}

}


extern short inp_log_push(uint32_t id, enum input_type type, uint32_t ts,
		void *in)
{
	short i;
	short k;
	short tmp;
	short from;
	short to;
	short ins = input.log.start;

	short islot = -1;
	short itr = -1;

	/* If the list is empty */
	if(input.log.num == 0) {	
		islot = 0;
		itr = 0;

		input.log.num = 1;
		goto insert;
	}

	/*
	 * Check if an input for an object with the same timestamp and type has
	 * already been made and overwrite it.
	 */
	for(i = 0; i < input.log.num; i++) {
		tmp = (input.log.start + i) % INP_LOG_LIM;

		if(input.log.ts[tmp] == ts && input.log.obj_id[tmp] == id) {
			if(input.log.type[tmp] == type) {
				switch(type) {
					case INP_T_NONE:
						break;

					case INP_T_MOV:
						vec2_cpy(input.log.mov[tmp], in);
						break;

					case INP_T_DIR:
						vec3_cpy(input.log.dir[tmp], in);
						break;
				}

				islot = tmp;
				itr = i;
				goto latest_update;
			}
		}
	}

	/* Attach entry to the end if possible */
	tmp = (input.log.start + input.log.num - 1) % INP_LOG_LIM;
	if(ts > input.log.ts[tmp]) {
		if(input.log.num + 1 > INP_LOG_LIM)
			input.log.start = (input.log.start + 1) % INP_LOG_LIM;
		else
			input.log.num += 1;

		islot = (tmp + 1) % INP_LOG_LIM;
		itr = input.log.num;
		goto insert;
	}

	/* Insert entry into the middle of the list */
	for(i = input.log.num - 1; i >= 0; i--) {
		tmp = (input.log.start + i) % INP_LOG_LIM;

		if(input.log.ts[tmp] <= ts) {
			ins = 0;

			if(input.log.num + 1 >= INP_LOG_LIM)
				ins = 1;

			/* Move all entries down one step */
			for(k = ins; k <= i; k++) {
				from = (input.log.start + k) % INP_LOG_LIM;

				to = from - 1;
				if(to < 0) to = INP_LOG_LIM + to;

				input.log.ts[to] = input.log.ts[from];
				input.log.type[to] = input.log.type[from];
				input.log.obj_id[to] = input.log.obj_id[from];

				vec2_cpy(input.log.mov[to], input.log.mov[from]);
				vec3_cpy(input.log.dir[to], input.log.dir[from]);
			}

			if(input.log.num + 1 < INP_LOG_LIM) {
				input.log.num += 1;

				input.log.start -= 1;
				if(input.log.start < 0)
					input.log.start = INP_LOG_LIM +
						input.log.start;
			}

			islot = tmp;
			itr = i + 1;
			goto insert;
		}
	}

	return -1;


insert:
	input.log.obj_id[islot] = id;
	input.log.ts[islot] = ts;
	input.log.type[islot] = type;

	switch(type) {
		case INP_T_NONE:
			break;

		case INP_T_MOV:
			vec2_cpy(input.log.mov[islot], in);
			break;

		case INP_T_DIR:
			vec3_cpy(input.log.dir[islot], in);
			break;
	}

latest_update:
	if(input.log.latest_itr == -1) {
		input.log.latest_slot = islot;
		input.log.latest_itr = itr;
		input.log.latest_ts = ts;
	}
	else {
		if(input.log.latest_ts > ts && input.log.latest_itr > itr) {
			input.log.latest_slot = islot;
			input.log.latest_itr = itr;
			input.log.latest_ts = ts;
		}
	}

	/*
	 * TODO: Remove unnecessary print
	 */
	inp_log_print();

	return islot;
}

extern int inp_check_new(void)
{
	if(input.log.latest_slot > -1)
		return 1;

	return 0;
}


extern void inp_log_print(void)
{
	short i;

	printf("Num: %d, Start: %d\n", input.log.num, input.log.start);

	for(i = 0; i < input.log.num; i++) {
		short slot = (input.log.start + i) % INP_LOG_LIM;

		printf("%2d(%2d): ", i, slot);

		printf("%8x >> ", input.log.obj_id[slot]);
		printf("type: %4d - ", input.log.type[slot]);
		printf("ts: %6x - ", input.log.ts[slot]);

		switch(input.log.type[slot]) {
			case INP_T_NONE:
				break;

			case INP_T_MOV:
				vec2_print(input.log.mov[slot]);
				break;

			case INP_T_DIR:
				vec3_print(input.log.dir[slot]);
				break;
		}


		if(slot == input.log.start)
			printf("  \t Start  ");

		if(slot == input.log.latest_slot)
			printf("  \t  !!");

		printf("\n");
	}
}


extern void inp_reset(void)
{
	input.log.latest_slot = -1;
	input.log.latest_ts = 0;
	input.log.itr = 0;
}


extern void inp_begin(void)
{
	if(input.log.latest_slot < 0)
		return;

	if(input.log.latest_slot >= input.log.start) {
		input.log.itr = input.log.latest_slot - input.log.start;	
	}
	else {
		input.log.itr = INP_LOG_LIM - (input.log.start -
				input.log.latest_slot);
	}
}


extern int inp_next(void)
{
	if(input.log.itr + 1 > input.log.num)
		return 0;

	input.log.itr += 1;
	return 1;
}


extern uint32_t inp_cur_ts(void)
{
	struct input_log *log = &input.log;

	if(log->num > 0) {
		short tmp = (log->start + log->itr) % INP_LOG_LIM;
		return log->ts[tmp];
	}

	return 0;
}

extern uint32_t inp_next_ts(void)
{
	struct input_log *log = &input.log;

	if(log->itr + 1 >= log->num)
		return 0;

	return log->ts[log->itr + 1];
}


extern int inp_get(struct input_entry *ent)
{
	int idx;
	struct input_log *log = &input.log;

	if(log->num < 1 || log->itr >= log->num)
		return 0;

	idx = (log->start + log->itr) % INP_LOG_LIM;

	ent->obj_id =  log->obj_id[idx];
	ent->type =    log->type[idx];
	ent->ts =      log->ts[idx];

	switch(log->type[idx]) {
		case INP_T_NONE:
			break;

		case INP_T_MOV:
			vec2_cpy(ent->mov, log->mov[idx]);
			break;

		case INP_T_DIR:
			vec3_cpy(ent->dir, log->dir[idx]);
			break;
	}

	return 1;
}


extern int inp_pack(char *out)
{
	short i;
	short s;
	short num;
	char *ptr = out;
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
	char i;
	char num;

	uint32_t  id;
	uint16_t  off;

	vec2_t   mov;
	vec3_t   cam;

	uint8_t type;
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
			case INP_T_MOV:
				vec2_cpy(mov, (float *)ptr);
				ptr += VEC2_SIZE;

				if(inp_push(INP_PIPE_IN, id, ts, type, mov) < 0)
					return -1;
				break;

			case INP_T_DIR:
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

extern void inp_proc(void)
{
	uint32_t ts;

	/* Check if the value really has changed */
	if((input.mask & 1) && !vec2_cmp(input.mov, input.mov_old)) {
		/* If yes, then save value and mark change */
		vec2_cpy(input.mov_old, input.mov);

		ts = ceil(input.mov_ts / TICK_TIME) * TICK_TIME;

		/* Push new entries into the in- and out-pipe */
		inp_push(INP_PIPE_IN, objects.id[core.obj],
				INP_T_MOV, ts, input.mov);
		inp_push(INP_PIPE_OUT, objects.id[core.obj],
				INP_T_MOV, ts, input.mov);
	}


	/* Check if the value really has changed */
	if((input.mask & 2) && !vec3_cmp(input.dir, input.dir_old)) {
		/* If yes, then save value and mark change */
		vec3_cpy(input.dir_old, input.dir);

		ts = ceil(input.dir_ts / TICK_TIME) * TICK_TIME;

		/* Push new entries into the in- and out-pipe */
		inp_push(INP_PIPE_IN, objects.id[core.obj],
				INP_T_DIR, ts, input.dir);
		inp_push(INP_PIPE_OUT, objects.id[core.obj],
				INP_T_DIR, ts, input.dir);
	}

	input.mask = 0;
}

extern void inp_update(uint32_t ts)
{
	struct input_entry inp;

	/* Sort the entries in both pipes from lowest to hights */
	inp_pipe_sort(INP_PIPE_IN);
	inp_pipe_sort(INP_PIPE_OUT);

	/*
	 * TODO: Validate input.
	 */


	input.log.latest_slot = -1;

	/* Push all new entries from the in-pipe into the input-log */
	while(inp_pull(&inp)) {
		void *ptr;
		short slot;

		switch(inp.type) {
			case INP_T_MOV:
				ptr = &inp.mov;
				break;

			case INP_T_DIR:
				ptr = &inp.dir;
				break;
		}

		if((slot = inp_log_push(inp.obj_id, inp.type, inp.ts, ptr)) < 0)
			continue;


		/*
		 * Update latest inputs.
		 */

		if(input.log.latest_slot < 0) {
			input.log.latest_slot = slot;
			input.log.latest_ts = inp.ts;
		}
		else {
			if(inp.ts < input.log.latest_ts) {
				input.log.latest_slot = slot;
				input.log.latest_ts = inp.ts;
			}
		}
	}
}
