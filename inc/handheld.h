#ifndef _HANDHELD_H
#define _HANDHELD_H

#include "vector.h"

#include <stdint.h>

#define HND_LIM           8
#define HND_NAME_MAX      32
#define HND_NAME_MAX_NT   (HND_NAME_MAX+1)

#define HND_HOOK_LIM      5


/* TODO */
struct hnd_handle {
	int i;
};


#define HND_M_NONE 0

struct hnd_wrapper {
	short num;

	uint16_t  mask[HND_LIM];
	char      name[HND_LIM][HND_NAME_MAX_NT];
	short     mdl[HND_LIM];

	/* The index of the parent-hook */
	short     par_hook[HND_LIM];

	/* 
	 * The position relative to the object and the barrel-direction in the
	 * resting-position.
	 */
	vec3_t    aim_pos[HND_LIM];
	vec3_t    aim_dir[HND_LIM];

	/* The vector and the matrix from the hook to the handheld */
	short     hook_c[HND_LIM];
	short     hook_idx[HND_LIM][HND_HOOK_LIM];
	vec3_t    hook_vec[HND_LIM][HND_HOOK_LIM];
	mat4_t    hook_mat[HND_LIM][HND_HOOK_LIM];
	mat4_t    hook_mat_inv[HND_LIM][HND_HOOK_LIM];
};


/* Define global handheld-wrapper */
extern struct hnd_wrapper g_hnd;


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
 * @tex_slot: The index of the texture in the texture-table
 * @shd_slot: The index of the shader in the shader-table
 *
 * Returns: The slot the handheld is put on or -1 if an error occurred
 */
extern short hnd_load(char *pth, short tex_slot, short shd_slot);


/*
 * Remove a handheld from the table and delete the associated model from the
 * model-table.
 *
 * @slot: The slot of the handheld in the table
 */
extern void hnd_remv(short slot);


#endif /* _HANDHELD_H */
