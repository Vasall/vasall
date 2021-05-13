#include "handheld.h"

#include "model.h"

#include <stdlib.h>


/* Redefine global handheld-wrapper */
struct hnd_wrapper g_hnd;


extern int hnd_init(void)
{
	int i;

	for(i = 0; HND_LIM; i++) {
		g_hnd.mask[i] = HND_M_NONE;
	}

	return 0;
}


static short hnd_get_slot(void)
{
	short i;

	for(i = 0; i < HND_LIM; i++) {
		if(g_hnd.mask[i] == HND_M_NONE)
			return i;
	}

	return -1;
}

extern short hnd_load(char *pth, short tex_slot, short shd_slot)
{
	FILE *fd;
	char cmd_buf[48];
	short slot;
	int i;

	/* Get a slot for the new handheld */
	if((slot = hnd_get_slot()) < 0)
		return -1;

	if(!(fd = fopen(pth, "r")))
		return -1;

	/*
	 * Reset the values of the slot.
	 */
	g_hnd.hook_c[slot] = 0;

	/*
	 * Read the handheld-data.
	 */
	while(fscanf(fd, "%s", cmd_buf) != EOF) {
		if(strcmp(cmd_buf, "end") == 0)
			break;

		/* hnd <name> */
		if(strcmp(cmd_buf, "hnd") == 0) {
			/* Read the name of the handheld */
			fscanf(fd, "%s",
					g_hnd.name[slot]);
		}
		/* phk <hook-index> */
		else if(strcmp(cmd_buf, "phk") == 0) {
			/* Read the hook the handheld is attached to */
			fscanf(fd, "%hd",
					&g_hnd.par_hook[slot]);
		}
		/* hok <index> <x> <y> z<> */
		else if(strcmp(cmd_buf, "hok") == 0) {
			short tmp;

			tmp = g_hnd.hook_c[slot];

			/* Read the index of the hook */
			fscanf(fd, "%hd", 
					&g_hnd.hook_idx[slot][tmp]);

			/* Read the vector */
			fscanf(fd, "%f %f %f",
					&g_hnd.hook_vec[slot][tmp][0],
					&g_hnd.hook_vec[slot][tmp][1],
					&g_hnd.hook_vec[slot][tmp][2]);

			/* Increment the number of hooks */
			g_hnd.hook_c[slot] += 1;
		}
		/* bof <x> <y> <z>*/
		else if(strcmp(cmd_buf, "bof") == 0) {
			fscanf(fd, "%f %f %f",
					&g_hnd.brl_off[slot][0],
					&g_hnd.brl_off[slot][1],
					&g_hnd.brl_off[slot][2]);
		}
	}

	/*
	 * Read the model-data and add it to the model-table.
	 */
	if((g_hnd.mdl[slot] = mdl_load_ffd(g_hnd.name[slot], fd, tex_slot,
					shd_slot, MDL_TYPE_DEFAULT)) < 0)
		goto err_remv_hnd;

	/*
	 * Go through the list of hooks and calculate the matrices from the
	 * vectors.
	 */
	for(i = 0; i < g_hnd.hook_c[slot]; i++) {
		mat4_idt(g_hnd.hook_mat[slot][i]);
		mat4_pfpos(g_hnd.hook_mat[slot][i], g_hnd.hook_vec[slot][i]);
	}

	fclose(fd);
	return slot;

err_remv_hnd:
	hnd_remv(slot);
	fclose(fd);

	return -1;
}


extern void hnd_remv(short slot)
{
	/* Reset the mask */
	g_hnd.mask[slot] = HND_M_NONE;
}
