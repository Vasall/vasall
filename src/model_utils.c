#include "model_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

/*
 * Loads a model from a wavefront .obj file.
 * The model gets added to the model cache and
 * the index is returned.
 *
 * @mdl: The model struct to set the mash of
 * @pth: The absolute path to the obj-file
 *
 * Returns: 0 on success, -1 on any error
 */
int mdlLoadObj(char *pth, int *idxnum, int **idx, int *vtxnum,
		Vec3 **vtx, Vec3 **nrm, Vec2 **uv)
{
	int ret = 0, i, j, tmp;
	FILE *fd;
	char char_buf[256];
	struct dyn_stack *vtx_in = NULL, *nrm_in = NULL, *tex_in = NULL;
	struct dyn_stack *vtx_out = NULL, *nrm_out = NULL, *tex_out = NULL;
	struct dyn_stack *idx_in = NULL, *idx_conv = NULL, *idx_out = NULL;

	/* Open the file in read-mode */
	if((fd = fopen(pth, "r")) == NULL) { 
		ret = -1; goto close; 
	}

	/* Create buffers to read input data into */
	if((vtx_in = stcCreate(VEC3_SIZE)) == NULL) goto failed;
	if((nrm_in = stcCreate(VEC3_SIZE)) == NULL) goto failed;
	if((tex_in = stcCreate(VEC2_SIZE)) == NULL) goto failed;

	/* Create buffers to write output data to */
	if((vtx_out = stcCreate(VEC3_SIZE)) == NULL) goto failed;
	if((nrm_out = stcCreate(VEC3_SIZE)) == NULL) goto failed;
	if((tex_out = stcCreate(VEC2_SIZE)) == NULL) goto failed;

	/* Create buffers to store the indices */
	if((idx_in = stcCreate(INT3)) == NULL) goto failed;
	if((idx_conv = stcCreate(INT3)) == NULL) goto failed;
	if((idx_out = stcCreate(sizeof(int))) == NULL) goto failed;

	/* Read the data from the obj-file */
	while(fscanf(fd, "%s", char_buf) != EOF) {
		if(strcmp(char_buf, "vt") == 0) {
			Vec2 tex_tmp;

			/* Read texture-data */
			fscanf(fd, "%f %f", 
					&tex_tmp[0], &tex_tmp[1]);

			/* Flip vertical-position */
			tex_tmp[1] = 1.0 - tex_tmp[1];

			stcPush(tex_in, &tex_tmp);

		}
		else if(strcmp(char_buf, "vn") == 0) {
			Vec3 nrm_tmp;

			/* Read the normal-vector */
			fscanf(fd, "%f %f %f", 
					&nrm_tmp[0], &nrm_tmp[1], &nrm_tmp[2]);

			/* Push the normal into the normal-stack */	
			stcPush(nrm_in, &nrm_tmp);
		}
		else if(strcmp(char_buf, "v") == 0) {
			Vec3 vtx_tmp;

			/* Read the vector-position */
			fscanf(fd, "%f %f %f", 
					&vtx_tmp[0], &vtx_tmp[1], &vtx_tmp[2]);

			/* Push the vertex into the vertex-array */
			stcPush(vtx_in, &vtx_tmp);
		}
		else if(strcmp(char_buf, "f") == 0) {
			int idx_tmp[3];

			/* Read the different indices */
			for(i = 0; i < 3; i++) {
				fscanf(fd, "%d/%d/%d", 
						&idx_tmp[0], &idx_tmp[1], &idx_tmp[2]);

				idx_tmp[0] -= 1;
				idx_tmp[1] -= 1;
				idx_tmp[2] -= 1;

				/* Push the indices into stack */
				stcPush(idx_in, idx_tmp);
			}
		}
	}

	/* Convert the data into a form usable for OpenGL */
	for(i = 0; i < idx_in->num; i++) {
		int cur[3], chk[3];
		uint8_t push = 1, same = 0;

		tmp = i;

		/* Copy the current indices into the cur-buffer */
		memcpy(cur, stcGet(idx_in, i), INT3);

		/* Check for similar vertices */
		for(j = 0; j < idx_conv->num; j++) {
			if(memcmp(cur, stcGet(idx_conv, j), INT3) == 0) {
				same = 1;

				tmp = j;
				stcPush(idx_out, &tmp);
				push = 0;

				break;
			}
		}

		if(!same) {
			for(j = 0; j < idx_conv->num; j++) {
				memcpy(chk, stcGet(idx_conv, j), INT3);

				/* If it's the same vertex */
				if(chk[0] == cur[0]) {
					/* Not the same normal-vector */
					if(chk[1] != cur[1] || chk[2] != cur[2]) {
						/* Push the vertex to the end */
						tmp = stcPush(idx_conv, cur);

						/* Push the index into the index_list */
						stcPush(idx_out, &tmp);

						/* Prevent pushing again */
						push = 0;
						break;
					}
				}
			}
		}

		/* If pushing is enabled */
		if(push) {
			/* Push the indices into the list */
			tmp = stcPush(idx_conv, cur);

			/* Push the index into the index_list */
			stcPush(idx_out, &tmp);
		}
	}

	/* Push the data into the different arrays */
	for(i = 0; i < idx_conv->num; i++) {
		int cur[3];
		Vec3 vtx_tmp, nrm_tmp;
		Vec2 tex_tmp;

		memcpy(cur, stcGet(idx_conv, i), INT3);

		memcpy(vtx_tmp, stcGet(vtx_in, cur[0]), VEC3_SIZE);
		memcpy(tex_tmp, stcGet(tex_in, cur[1]), VEC2_SIZE);
		memcpy(nrm_tmp, stcGet(nrm_in, cur[2]), VEC3_SIZE);

		stcPush(vtx_out, vtx_tmp);
		stcPush(tex_out, tex_tmp);
		stcPush(nrm_out, nrm_tmp);
	}

	*idxnum = idx_out->num;

	*idx = malloc(*idxnum * sizeof(int));
	memcpy(*idx, idx_out->buf, idx_out->num * sizeof(int));

	*vtxnum = idx_conv->num;

	*vtx = malloc(*vtxnum * VEC3_SIZE);
	memcpy(*vtx, vtx_out->buf, vtx_out->num * VEC3_SIZE);
	
	*nrm = malloc(*vtxnum * VEC3_SIZE);
	memcpy(*nrm, nrm_out->buf, nrm_out->num * VEC3_SIZE);

	*uv = malloc(*vtxnum * VEC2_SIZE);
	memcpy(*uv, tex_out->buf, tex_out->num * VEC2_SIZE);

	goto cleanup;

failed:
	ret = -1;

cleanup:
	stcDestroy(vtx_in);
	stcDestroy(nrm_in);
	stcDestroy(tex_in);

	stcDestroy(vtx_out);
	stcDestroy(nrm_out);
	stcDestroy(tex_out);

	stcDestroy(idx_in);
	stcDestroy(idx_conv);
	stcDestroy(idx_out);

	fclose(fd);

close:
	return (ret);
}

