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
 * @pth: The path to the models .obj file
 *
 * Returns: 0 on success, -1 on any error
 */
int mdlLoad(struct model *mdl, char *pth)
{
	int ret = 0, i;
	FILE *fd;
	char char_buf[256];
	struct dyn_stack *vertices, *normals, *textures;
	struct dyn_stack *indices, *normal_indices, *tex_indices;

	if(mdl == NULL) return(-1);

	fd = fopen(pth, "r");

	if((vertices = stcCreate(VEC3_SIZE)) == NULL) { ret = -1; goto cleanup; }
	if((normals = stcCreate(VEC3_SIZE)) == NULL) { ret = -1; goto cleanup; }
	if((textures = stcCreate(VEC2_SIZE)) == NULL)  { ret = -1; goto cleanup; }
	
	if((indices = stcCreate(sizeof(int))) == NULL)  { ret = -1; goto cleanup; }
	if((normal_indices = stcCreate(sizeof(int))) == NULL) { ret = -1; goto cleanup; }
	if((tex_indices = stcCreate(sizeof(int))) == NULL)  { ret = -1; goto cleanup; }

	while(fscanf(fd, "%s", char_buf) != EOF) {
		if(strcmp(char_buf, "vt") == 0) {
			float f;
			/* texture data */
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

			printf("%d: %1.2f/%1.2f/%1.2f\n", vertices->num,
					vtx[0], vtx[1], vtx[2]);

			/* Push the vertex into the vertex-array */
			stcPush(vertices, &vtx);
		}
		else if(strcmp(char_buf, "f") == 0) {
			int idx[3];

			/* Read the different indices */
			for(i = 0; i < 3; i++) {
				fscanf(fd, "%d/%d/%d", 
						&idx[0], &idx[1], &idx[2]);
			
				idx[0] -= 1;

				/* Push the indices into the proper stacks */
				stcPush(indices, &idx[0]);
				stcPush(normal_indices, &idx[1]);
				stcPush(tex_indices, &idx[2]);
			}
			
		}
	}

#ifdef DEBUF
	for(i = 0; i < indices->num; i++) {
		float vtx[3];
		int idx = ((int *)indices->buf)[i];
	
		memcpy(vtx, (float *)vertices->buf + idx * 3 * sizeof(float),
				sizeof(float) * 3);

		printf("%d: %d (%1.2f/%1.2f/%1.2f)\n", i, idx,
				vtx[0], vtx[1], vtx[2]);
	}
#endif

	mdlSetMesh(mdl, vertices->buf, vertices->num, 
			(uint32_t *)indices->buf, indices->num, 1);

cleanup:
	stcDestroy(vertices);
	stcDestroy(normals);
	stcDestroy(textures);
	stcDestroy(indices);
	stcDestroy(normal_indices);
	stcDestroy(tex_indices);
	
	fclose(fd);

	return (ret);
}
