#include "mdl_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"
#include "vec.h"
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
int mdlLoadObj(struct model *mdl, char *pth)
{
	int ret = 0, i, j, tmp;
	FILE *fd;
	char char_buf[256];
	struct dyn_stack *vtx_in = NULL, *nrm_in = NULL, *tex_in = NULL;
	struct dyn_stack *vtx_out = NULL, *nrm_out = NULL, *tex_out = NULL;
	struct dyn_stack *idx_in = NULL, *idx_conv = NULL, *idx_out = NULL;

	if(mdl == NULL || mdl->status != MDL_OK) return(-1);

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
			float f;

			/* Texture data */
			fscanf(fd, "%f", &f);
			fscanf(fd, "%f", &f);

		}
		else if(strcmp(char_buf, "vn") == 0) {
			Vec3 nrm;

			/* Read the normal-vector */
			fscanf(fd, "%f %f %f", 
					&nrm[0], &nrm[1], &nrm[2]);

			/* Push the normal into the normal-stack */	
			stcPush(nrm_in, &nrm);
		}
		else if(strcmp(char_buf, "v") == 0) {
			Vec3 vtx;

			/* Read the vector-position */
			fscanf(fd, "%f %f %f", 
					&vtx[0], &vtx[1], &vtx[2]);

			/* Push the vertex into the vertex-array */
			stcPush(vtx_in, &vtx);
		}
		else if(strcmp(char_buf, "f") == 0) {
			int idx[3];

			/* Read the different indices */
			for(i = 0; i < 3; i++) {
				fscanf(fd, "%d/%d/%d", 
						&idx[0], &idx[1], &idx[2]);

				/* Obj-indices start at 1 */	
				for(j = 0; j < 3; j++) {
					idx[j] -= 1;
				}

				/* Push the indices into stack */
				stcPush(idx_in, idx);
			}
		}
	}

	/* Convert the data into a form usable for OpenGL */
	for(i = 0; i < idx_in->num; i++) {
		int cur[3], chk[3];
		uint8_t push = 1, same = 0;
		void *ptr;

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
					if(chk[2] != cur[2] || 
							chk[2] != cur[2]) {
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
		Vec3 vtx, nrm;

		memcpy(cur, stcGet(idx_conv, i), INT3);

		memcpy(vtx, stcGet(vtx_in, cur[0]), VEC3_SIZE);
		memcpy(nrm, stcGet(nrm_in, cur[2]), VEC3_SIZE);

		stcPush(vtx_out, vtx);
		stcPush(nrm_out, nrm);
	}

	mdlSetMesh(mdl, vtx_out->buf, vtx_out->num, 
			(uint32_t *)idx_out->buf, idx_out->num, 0);

	mdlAddBAO(mdl, 0, nrm_out->buf, VEC3_SIZE, nrm_out->num, 
			1, 3, 0, "vtxNrm");

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

close:
	fclose(fd);

	/* Mark the model as failed */
	if(ret < 0) mdl->status = MDL_ERR_LOADING;

	return (ret);
}
