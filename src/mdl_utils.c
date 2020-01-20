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
	int ret = 0, i, j;
	FILE *fd;
	char char_buf[256];
	struct dyn_stack *vertices, *normals, *textures;
	struct dyn_stack *indices, *normal_indices, *tex_indices;

	if(mdl == NULL || mdl->status != MDL_OK) return(-1);

	/* Open the file in read-mode */
	if((fd = fopen(pth, "r")) == NULL) { 
		ret = -1; goto close; 
	}

	if((vertices = stcCreate(VEC3_SIZE)) == NULL) { 
		ret = -1; goto cleanup; 
	}
	if((normals = stcCreate(VEC3_SIZE)) == NULL) { 
		ret = -1; goto cleanup; 
	}
	if((textures = stcCreate(VEC2_SIZE)) == NULL)  { 
		ret = -1; goto cleanup;
	}

	if((indices = stcCreate(sizeof(int))) == NULL)  { 
		ret = -1; goto cleanup; 
	}
	if((normal_indices = stcCreate(sizeof(int))) == NULL) { 
		ret = -1; goto cleanup; 
	}
	if((tex_indices = stcCreate(sizeof(int))) == NULL) { 
		ret = -1; goto cleanup; 
	}

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
			stcPush(normals, &nrm);
		}
		else if(strcmp(char_buf, "v") == 0) {
			Vec3 vtx;

			/* Read the vector-position */
			fscanf(fd, "%f %f %f", 
					&vtx[0], &vtx[1], &vtx[2]);

			/* Push the vertex into the vertex-array */
			stcPush(vertices, &vtx);
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

				/* Push the indices into the proper stacks */
				stcPush(indices, &idx[0]);
				stcPush(normal_indices, &idx[1]);
				stcPush(tex_indices, &idx[2]);
			}
		}
	}

	mdlSetMesh(mdl, vertices->buf, vertices->num, 
			(uint32_t *)indices->buf, indices->num, 1);

cleanup:
	stcDestroy(vertices);
	stcDestroy(normals);
	stcDestroy(textures);
	stcDestroy(indices);
	stcDestroy(normal_indices);
	stcDestroy(tex_indices);

close:
	fclose(fd);

	/* Mark the model as failed */
	if(ret < 0) mdl->status = MDL_ERR_LOADING;

	return (ret);
}
