#ifndef _HANDHELD_H
#define _HANDHELD_H

#include <stdint.h>

#define HND_LIM           8
#define HND_NAME_MAX      32
#define HDN_NAME_MAX_NT   (HND_NAME_MAX+1)


struct handheld_wrapper {
	short num;

	uint16_t  mask[HND_LIM];
	char      name[HND_LIM][HND_NAME_MAX_NT];	
	short     mdl[HDN_LIM];
};


/* Define global handheld-wrapper */
extern struct handheld_wrapper handhelds;


/*
 * Initialize the handheld table.
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int hnd_init(void);


/*
 * Load a new handheld-object into the table and add the associated model into
 * the model-table.
 *
 * @pth: The path to the file containing the handheld-data
 *
 * Returns: The slot the handheld is put on or -1 if an error occurred
 */
extern short hnd_load(char *pth);


/*
 * Remove a handheld from the table and delete the associated model from the
 * model-table.
 *
 * @slot: The slot of the handheld in the table
 */
extern void hnd_rmv(short slot);


#endif /* _HANDHELD_H */
