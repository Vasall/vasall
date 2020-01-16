#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj_utils.h"
#include "vec.h"

/*
 * Loads a model from a wavefront .obj file.
 * The model gets added to the model cache and
 * the index is returned.
 *
 * @pth: The path to the models .obj file
 *
 * Returns: The new models index in the model cache,
 * or -1 on any error
 */
int ldMdl(char *pth)
{
	FILE *fd = fopen(pth, "r");
	char *line_buf = malloc(256 * sizeof(char));

	Vec3 *vertex_buf = malloc(BUF_ALLOC_STEP * VEC3_SIZE);
	int *index_buf = malloc(BUF_ALLOC_STEP * sizeof(int));
	Vec3 *normal_buf = malloc(BUF_ALLOC_STEP * VEC3_SIZE);

	float f0, f1, f2;

	while(fscanf(fd, "%s", line_buf) != EOF) {
		if(strstr(line_buf, "vt") != NULL) {
			/* texture data */
			
		} else if(strstr(line_buf, "vn") != NULL) {
			/* normal data*/

		} else if(strstr(line_buf, "v") != NULL) {
			/* vector data*/

		} else if(strstr(line_buf, "f")) {
			/* Index data */

		}
	}

	free(line_buf);

	return 0;
}
