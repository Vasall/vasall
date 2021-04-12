#include "handheld.h"

#include "model.h"

#include <stdlib.h>


/* Redefine global handheld-wrapper */
struct handheld_wrapper handhelds;


extern int hnd_init(void)
{
	int i;

	for(i = 0; HND_LIM; i++) {
		handhelds.mask[i] = HND_M_NONE;
	}

	return 0;
}


static short hnd_get_slot(void)
{
	short i;

	for(i = 0; i < HND_LIM; i++) {
		if(handhelds.mask[i] == HND_M_NONE)
			return i;
	}

	return -1;
}

extern short hnd_load(char *pth, short tex_slot, short shd_slot)
{
	FILE *fd;
	char cmd_buf[48];
	short slot;


	/* Get a slot for the new handheld */
	if((slot = hnd_get_slot()) < 0)
		return -1;

	if(!(fd = fopen(pth, "r")))
		return -1;

	while(fscanf(fd, "%s", cmd_buf) != EOF) {
		if(strcmp(cmd_buf, "end") == 0)
			break;

		if(strcmp(cmd_buf, "hnd") == 0) {
			/* Read the name of the handheld */
			fscanf(fd, "%s",
					handhelds.name[slot]);
		}
		if(strcmp(cmd_buf, "phk") == 0) {
			/* Read the hook the handheld is attached to */
			fscanf(fd, "%hd",
					&handhelds.hook[slot]);
		}
		if(strcmp(cmd_buf, "apo") == 0) {
			vec3_t tmp;

			/* Read the barrel-position */
			fscanf(fd, "%f %f %f",
					&tmp[0],
					&tmp[1],
					&tmp[2]);

			vec3_cpy(handhelds.aim_pos[slot], tmp);
		}
		if(strcmp(cmd_buf, "adi") == 0) {
			vec3_t tmp;

			/* Read the barrel-direction */
			fscanf(fd, "%f %f %f",
					&tmp[0],
					&tmp[1],
					&tmp[2]);

			vec3_cpy(handhelds.aim_dir[slot], tmp);
		}
	}
					
	if((handhelds.mdl[slot] = mdl_load_ffd(handhelds.name[slot], fd,
					tex_slot, shd_slot, MDL_TYPE_DEFAULT)) < 0)
		goto err_remv_hnd;

	printf("New slot %d\n", handhelds.mdl[slot]);

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
	handhelds.mask[slot] = HND_M_NONE;
}
