#include "mdl_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"
#include "vec.h"

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
	FILE *fd = fopen(pth, "r");
	char *char_buf = malloc(256 * sizeof(char));

	Vec3 *vertex_buf = malloc(BUF_ALLOC_STEP * VEC3_SIZE);
	int vertex_count = 0;

	Vec3 *normal_buf = malloc(BUF_ALLOC_STEP * VEC3_SIZE);
	int normal_count = 0;	

	Vec2 *tex_buf = malloc(BUF_ALLOC_STEP * VEC2_SIZE);
	int tex_count = 0;

	int *index_buf = malloc(BUF_ALLOC_STEP * sizeof(int));
	int *normal_index_buf = malloc(BUF_ALLOC_STEP * sizeof(int));
	int *tex_index_buf = malloc(BUF_ALLOC_STEP * sizeof(int));
	int index_count = 0;

	int i;
	int n_sz;

	if(mdl == NULL) {
		/* TODO check before allocating */
		printf("Model was NULL!\n");
		fclose(fd);
		free(char_buf);
		free(vertex_buf);
		free(normal_buf);
		free(tex_buf);
		free(index_buf);
		free(normal_index_buf);
		free(tex_index_buf);
		return -1;
	}

	while(1) {
		fscanf(fd, "%s", char_buf);
		if(strncmp(char_buf, "o", 1) == 0)
			break;
	}

	while(fscanf(fd, "%s", char_buf) != EOF) {
		if(strstr(char_buf, "vt") == char_buf) {
			float f;
			/* texture data */
			fscanf(fd, "%f", &f);
			fscanf(fd, "%f", &f);

		} else if(strstr(char_buf, "vn") == char_buf) {
			/* normal data*/
			fscanf(fd, "%f %f %f", &normal_buf[normal_count][0],
					&normal_buf[normal_count][1],
					&normal_buf[normal_count][2]);
			
			normal_count++;
			if(normal_count % BUF_ALLOC_STEP == 0) {
				n_sz = normal_count / BUF_ALLOC_STEP + 1;
				n_sz *= BUF_ALLOC_STEP;
				normal_buf = realloc(normal_buf,
						n_sz * VEC3_SIZE);
			}

		} else if(strstr(char_buf, "v") == char_buf) {
			/* vertex data*/
			fscanf(fd, "%f %f %f", &vertex_buf[vertex_count][0],
					&vertex_buf[vertex_count][1],
					&vertex_buf[vertex_count][2]);
			
			vertex_count++;
			if(vertex_count % BUF_ALLOC_STEP == 0) {
				n_sz = vertex_count / BUF_ALLOC_STEP + 1;
				n_sz *= BUF_ALLOC_STEP;
				vertex_buf = realloc(vertex_buf,
						n_sz * VEC3_SIZE);
			}


		} else if(strstr(char_buf, "f") == char_buf) {
			/* index data */
			for(i = 0; i < 3; i++) {
				fscanf(fd, "%d/%d/%d",
						&index_buf[index_count],
						&tex_index_buf[index_count],
						&normal_index_buf[index_count]);
				
				index_count++;
				if(index_count % BUF_ALLOC_STEP == 0) {
					n_sz = index_count / BUF_ALLOC_STEP + 1;
					n_sz *= BUF_ALLOC_STEP;
	
					index_buf = realloc(index_buf,
							n_sz * sizeof(int));
					normal_index_buf = realloc(normal_index_buf,
							n_sz * sizeof(int));
					tex_index_buf = realloc(tex_index_buf,
							n_sz * sizeof(int));
				}
			}
			
		}
	}

	printf("Finished parsing %s\n", pth);

	mdlSetMesh(mdl, vertex_buf, vertex_count, (uint32_t *) index_buf, index_count, 1);

	fclose(fd);
	free(char_buf);

	return 0;
}
