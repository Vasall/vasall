#include "amoloader.h"

#include "array.h"
#include <stdint.h>



AMO_API struct amo_model *amo_load(const char *pth)
{
	int i;
	int j;
	int tmp;
	void *p;

	FILE *fd;
	struct amo_model *data;

	char cmd_buf[128];
	enum amo_format format = AMO_FORMAT_NONE;
	uint32_t attr_m = AMO_M_NONE;

	int vtx_num =     200;
	int tex_num  =    200;
	int nrm_num =     200;
	int vjnt_num =    200;
	int wgt_num =     200;
	int jnt_num =     200;
	int ani_num =     200;
	int keyfr_num =    20;
	int idx_num =     200;
	int cm_vtx_num =  200;
	int cm_idx_num =  200;

	/* Check if file is has the right extension */
	if(strcmp(strrchr(pth, '.'), ".amo") == 0)
		;
	else if(strcmp(strrchr(pth, '.'), ".obj") == 0)
		;
	else
		return NULL;

	/* Try to open the file */
	if(!(fd = fopen(pth, "r")))
		return NULL;

	/* 
	 * Allocate memory for the data-struct.
	 * Note that as we use calloc for allocating memory, all bytes will be
	 * set to 0, and therefore we dont have to initialize the attributes.
	 */
	if(!(data = calloc(sizeof(struct amo_model), 1)))
		goto err_close_file;

	/*
	 * Search for the format-type.
	 *
	 * <format-type> <model-name>
	 */
	while(fscanf(fd, "%s", cmd_buf) != EOF) {
		if(strcmp(cmd_buf, "o") == 0) {
			format = AMO_FORMAT_OBJ;
			break;
		}
		else if(strcmp(cmd_buf, "ao") == 0) {
			format = AMO_FORMAT_AMO;
			break;
		}
	}

	/*
	 * If no format-type has been found, then terminate.
	 */
	if(format == AMO_FORMAT_NONE)
		return NULL;

	/*
	 * Read the model-name.
	 */
	fscanf(fd, "%s", data->name);

	/* Read the data-mask */
	if(format == AMO_FORMAT_AMO)
		fscanf(fd, "%u", &attr_m);

	data->attr_m = attr_m;

	/* 
	 * Go through every line and read the first word. If it's a valid amo
	 * keyword read the arguments and put them into the right position in
	 * the data struct.
	 */
	while(fscanf(fd, "%s", cmd_buf) != EOF) {
		/* v <x> <y> <z> */
		if(strcmp(cmd_buf, "v") == 0) {
			/* Increment the number of vertices */
			data->vtx_c++;

			/* Allocate memory if necessary */
			if(data->vtx_c == 1) {
				vtx_num = 200;
				tmp = sizeof(float) * 3 * vtx_num;

				if(!(data->vtx_buf = calloc(1, tmp)))
					goto err_free_data;
			}
			else if(data->vtx_c > vtx_num) {
				vtx_num *= 1.5;
				tmp = sizeof(float) * 3 * vtx_num;

				if(!(p = realloc(data->vtx_buf, tmp)))
					goto err_free_data;

				data->vtx_buf = p;
			}

			/* Read the data of the new vertex */
			tmp = (data->vtx_c - 1) * 3;
			fscanf(fd, "%f %f %f",
					&data->vtx_buf[tmp + 0],
					&data->vtx_buf[tmp + 1],
					&data->vtx_buf[tmp + 2]);
		}
		/* vt <x> <y> */
		else if(strcmp(cmd_buf, "vt") == 0) {
			/* Increment the number of texture-coordinates */
			data->tex_c++;

			/* Allocate memory if necessary */
			if(data->tex_c == 1)  {
				tex_num = 200;
				tmp = sizeof(float) * 2 * tex_num;

				if(!(data->tex_buf = calloc(1, tmp)))
					goto err_free_data;
			}
			else if(data->tex_c > tex_num) {
				tex_num *= 1.5;
				tmp = sizeof(float) * 2 * tex_num;

				if(!(p = realloc(data->tex_buf, tmp)))
					goto err_free_data;

				data->tex_buf = p;
			}

			/* Read the data of the new uv-coordinate */
			tmp = (data->tex_c - 1) * 2;
			fscanf(fd, "%f %f",
					&data->tex_buf[tmp + 0],
					&data->tex_buf[tmp + 1]);

			/* Flip uv-coordinates */
			data->tex_buf[tmp + 1] = 1.0 - data->tex_buf[tmp + 1];
		}
		/* vn <x> <y> <z> */
		else if(strcmp(cmd_buf, "vn") == 0) {
			/* Increment the number of normal-vectors */
			data->nrm_c++;

			/* Allocate memory if necessary */
			if(data->nrm_c == 1) {
				nrm_num = 200;
				tmp = sizeof(float) * 3 * nrm_num;

				if(!(data->nrm_buf = calloc(1, tmp)))
					goto err_free_data;
			}
			else if(data->nrm_c > nrm_num) {
				nrm_num *= 1.5;
				tmp = sizeof(float) * 3 * nrm_num;

				if(!(p = realloc(data->nrm_buf, tmp)))
					goto err_free_data;

				data->nrm_buf = p;
			}

			/* Read the data of the new normal-vector */
			tmp = (data->nrm_c - 1) * 3;
			fscanf(fd, "%f %f %f",
					&data->nrm_buf[tmp + 0],
					&data->nrm_buf[tmp + 1],
					&data->nrm_buf[tmp + 2]);
		}
		/* vj <joint_1> <joint_2> <joint_3> <joint_4> */
		else if(strcmp(cmd_buf, "vj") == 0) {
			/* Increment the number of vertex-joints */
			data->vjnt_c++;

			/* Allocate memory if necessary */	
			if(data->vjnt_c == 1) {
				vjnt_num = 200;
				tmp = sizeof(int) * 4 * vjnt_num;

				if(!(data->vjnt_buf = calloc(1, tmp)))
					goto err_free_data;
			}
			else if(data->vjnt_c > vjnt_num) {
				vjnt_num *= 1.5;
				tmp = sizeof(int) * 4 * vjnt_num;

				if(!(p = realloc(data->vjnt_buf, tmp)))
					goto err_free_data;

				data->vjnt_buf = p;
			}

			/* Read the data of the vertex-joint */
			tmp = (data->vjnt_c - 1) * 4;
			fscanf(fd, "%d %d %d %d",
					&data->vjnt_buf[tmp + 0],
					&data->vjnt_buf[tmp + 1],
					&data->vjnt_buf[tmp + 2],
					&data->vjnt_buf[tmp + 3]);
		}
		/* vw <weight_1> <weight_2> <weight_3> <weight_4> */
		else if(strcmp(cmd_buf, "vw") == 0) {
			/* Increment the number of joints */
			data->wgt_c++;

			/* Allocate memory if necessary */	
			if(data->wgt_c == 1) {
				wgt_num = 200;
				tmp = sizeof(float) * 4 * wgt_num;

				if(!(data->wgt_buf = calloc(1, tmp)))
					goto err_free_data;
			}
			else if(data->wgt_c > wgt_num) {
				wgt_num *= 1.5;
				tmp = sizeof(float) * 4 * wgt_num;

				if(!(p = realloc(data->wgt_buf, tmp)))
					goto err_free_data;

				data->wgt_buf = p;
			}

			/* Read the data of the new weight */
			tmp = (data->wgt_c - 1) * 4;
			fscanf(fd, "%f %f %f %f",
					&data->wgt_buf[tmp + 0],
					&data->wgt_buf[tmp + 1],
					&data->wgt_buf[tmp + 2],
					&data->wgt_buf[tmp + 3]);
		}
		/* 
		 * f <pos>/<tex>/<normal>/<joint>/<weight>
		 *   <pos>/<tex>/<normal>/<joint>/<weight>
		 *   <pos>/<tex>/<normal>/<joint>/<weight>
		 */
		else if(strcmp(cmd_buf, "f") == 0) {
			int num = (attr_m & AMO_M_RIG) ? 5 : 3;

			/* Increment the number of indices */
			data->idx_c++;

			/* Allocate memory if necessary */	
			if(data->idx_c == 1) {
				idx_num = 200;
				tmp = sizeof(unsigned int) * (3 * num) * idx_num;

				if(!(data->idx_buf = calloc(1, tmp)))
					goto err_free_data;
			}
			else if(data->idx_c > idx_num) {
				idx_num *= 1.5;
				tmp = sizeof(unsigned int) * (3 * num) * idx_num;

				if(!(p = realloc(data->idx_buf, tmp)))
					goto err_free_data;

				data->idx_buf = p;
			}

			/* Read the indices in blocks of 3 with 5 each*/
			for(j = 0; j < 3; j++) {
				if(attr_m & AMO_M_RIG) {
					tmp = (data->idx_c - 1) * (3 * num) + (j * num);
					fscanf(fd, "%u/%u/%u/%u/%u",
							&data->idx_buf[tmp + 0],
							&data->idx_buf[tmp + 1],
							&data->idx_buf[tmp + 2],
							&data->idx_buf[tmp + 3],
							&data->idx_buf[tmp + 4]);

					/* OBJ-indices start counting at 1 */
					data->idx_buf[tmp + 0] -= 1;
					data->idx_buf[tmp + 1] -= 1;
					data->idx_buf[tmp + 2] -= 1;
					data->idx_buf[tmp + 3] -= 1;
					data->idx_buf[tmp + 4] -= 1;
				}
				else {
					tmp = ((data->idx_c - 1) * (3 * num)) + (j * num);
					fscanf(fd, "%u/%u/%u",
							&data->idx_buf[tmp + 0],
							&data->idx_buf[tmp + 1],
							&data->idx_buf[tmp + 2]);


					/* OBJ-indices start counting at 1 */
					data->idx_buf[tmp + 0] -= 1;
					data->idx_buf[tmp + 1] -= 1;
					data->idx_buf[tmp + 2] -= 1;

				}
			}
		}
		/* j <name> <parent> */
		else if(strcmp(cmd_buf, "j") == 0) {
			int joint_par;
			struct amo_joint *joint_tmp;

			/* Increment the number of joints */
			data->jnt_c++;

			/* Allocate memory if necessary */
			if(data->jnt_c == 1) {
				jnt_num = 200;
				tmp = sizeof(struct amo_joint) * jnt_num;

				if(!(data->jnt_lst = calloc(1, tmp)))
					goto err_free_data;
			}
			else if(data->jnt_c > jnt_num) {
				jnt_num *= 1.5;
				tmp = sizeof(struct amo_joint) * jnt_num;

				if(!(p = realloc(data->jnt_lst, tmp)))
					goto err_free_data;
				data->jnt_lst = p;
			}

			/* Read the data of the new joint */
			tmp = data->jnt_c - 1;
			fscanf(fd, "%s %d",
					data->jnt_lst[tmp].name,
					&joint_par);

			for(i = 0; i < 16; i++)
				fscanf(fd, "%f",
						&data->jnt_lst[tmp].mat[i]);

			/* Special treatment for root joint */
			if(joint_par == -1) {
				joint_tmp = NULL;
			}
			else {
				/* Correct the parent-index */
				joint_par -= 1;

				/* Get reference to parent-joint */
				joint_tmp = &data->jnt_lst[joint_par];
			}

			/* TODO */
			/* Set the parent-joint */
			data->jnt_lst[tmp].index = tmp;
			data->jnt_lst[tmp].par = joint_tmp;
		}
		/* a <name> */
		else if(strcmp(cmd_buf, "a") == 0) {
			/* Increment the number of animations */
			data->ani_c++;

			/* Allocate memory if necessary */
			if(data->ani_c == 1) {
				ani_num = 200;
				tmp = sizeof(struct amo_anim ) * ani_num;

				if(!(data->ani_lst = calloc(1, tmp)))
					goto err_free_data;
			}	
			else if(data->ani_c > ani_num) {
				ani_num *= 1.5;
				tmp = sizeof(struct amo_anim) * ani_num;

				if(!(p = realloc(data->ani_lst, tmp)))
					goto err_free_data;

				data->ani_lst = p;
			}

			/*  Set the number of keyframes to 0 */
			data->ani_lst[data->ani_c - 1].keyfr_c = 0;

			/* Read the data of the animation */
			tmp = data->ani_c - 1;
			fscanf(fd, "%s %d",
					data->ani_lst[tmp].name,
					&data->ani_lst[tmp].dur);
		}
		/* k <timestamp> */
		else if(strcmp(cmd_buf, "k") == 0) {
			struct amo_anim *ani = &data->ani_lst[data->ani_c - 1];
			short jnt_num;

			ani->keyfr_c++;

			/* Allocate memory if necessary */
			if(ani->keyfr_c == 1) {
				keyfr_num = 20;
				tmp = sizeof(struct amo_keyfr) * keyfr_num;

				if(!(ani->keyfr_lst = calloc(1, tmp)))
					goto err_free_data;
			}	
			else if(ani->keyfr_c > keyfr_num) {
				keyfr_num *= 1.5;
				tmp = sizeof(struct amo_keyfr) * keyfr_num;

				if(!(p = realloc(ani->keyfr_lst, tmp)))
					goto err_free_data;

				ani->keyfr_lst = p;
			}

			/* Read the data of the keyframe */
			tmp = ani->keyfr_c - 1;
			fscanf(fd, "%f %d",
					&ani->keyfr_lst[tmp].prog,
					&ani->keyfr_lst[tmp].jnt_num);

			jnt_num = ani->keyfr_lst[tmp].jnt_num;

			/* Allocate memory for all bones in the keyframe */
			tmp = jnt_num * sizeof(struct amo_joint *);
			if(!(ani->keyfr_lst[ani->keyfr_c - 1].joints = malloc(tmp)))
				goto err_free_data;

			tmp = jnt_num * sizeof(short);
			if(!(ani->keyfr_lst[ani->keyfr_c - 1].jnt = malloc(tmp)))
				goto err_free_data;

			for(i = 0; i < jnt_num; i++)
				ani->keyfr_lst[ani->keyfr_c - 1].jnt[i] = -1;

			tmp = jnt_num * 3 * sizeof(float);
			if(!(ani->keyfr_lst[ani->keyfr_c - 1].pos = malloc(tmp)))
				goto err_free_data;

			tmp = jnt_num * 4 * sizeof(float);
			if(!(ani->keyfr_lst[ani->keyfr_c - 1].rot = malloc(tmp)))
				goto err_free_data;
		}
		/* ap <joint> <x> <y> <z> */
		else if(strcmp(cmd_buf, "ap") == 0) {
			int joint;
			struct amo_anim *ani = &data->ani_lst[data->ani_c - 1];
			struct amo_keyfr *keyfr;
			int x;
			int found = -1;

			/* Get a pointer to the keyframe */
			tmp = ani->keyfr_c - 1;
			keyfr = &ani->keyfr_lst[tmp];

			/* Read the data of the position-keyframe */
			fscanf(fd, "%d",
					&joint);
		
			/* Indices start at 1 */
			joint -= 1;

			/* Either search for the joint-slot or a free-slot */
			for(x = 0; x < keyfr->jnt_num; x++) {
				if(keyfr->jnt[x] == joint) {
					found = x;
					break;
				}
				if(keyfr->jnt[x] == -1) {
					found = x;

					/* Claim free slot for joint  */
					keyfr->jnt[x] = joint;

					break;
				}
			}


			/* Set a pointer to the referenced joint */
			keyfr->joints[found] = &data->jnt_lst[joint];

			/* Read the rest of the position-keyframe */
			tmp = found * 3;
			fscanf(fd, "%f %f %f",
					&keyfr->pos[tmp + 0],
					&keyfr->pos[tmp + 1],
					&keyfr->pos[tmp + 2]);
		}
		/* ar <joint> <x> <y> <z> <w> */
		else if(strcmp(cmd_buf, "ar") == 0) {
			int joint;
			struct amo_anim *ani = &data->ani_lst[data->ani_c - 1];
			struct amo_keyfr *keyfr;
			int x;
			int found = -1;

			/* Get a pointer to the keyframe */
			tmp = ani->keyfr_c - 1;
			keyfr = &ani->keyfr_lst[tmp];

			/* Read the data of the position-keyframe */
			fscanf(fd, "%d",
					&joint);

			/* Indices start at 1 */
			joint -= 1;

			/* Either search for the joint-slot or a free-slot */
			for(x = 0; x < keyfr->jnt_num; x++) {
				if(keyfr->jnt[x] == joint) {
					found = x;
					break;
				}
				else if(keyfr->jnt[x] == -1) {
					found = x;

					/* Claim free slot for joint  */
					keyfr->jnt[x] = joint;

					break;
				}
			}

			/* Set a pointer to the referenced joint */
			keyfr->joints[found] = &data->jnt_lst[joint];

			/* Read the rest of the position-keyframe */
			tmp = found * 4;
			fscanf(fd, "%f %f %f %f",
					&keyfr->rot[tmp + 0],
					&keyfr->rot[tmp + 1],
					&keyfr->rot[tmp + 2],
					&keyfr->rot[tmp + 3]);
		}
		/* bp <x> <y> <z> <sx> <sy> <sz> */
		else if(strcmp(cmd_buf, "bp") == 0) {
			fscanf(fd, "%f %f %f %f %f %f",
					&data->bp_col.pos[0],
					&data->bp_col.pos[1],
					&data->bp_col.pos[2],
					&data->bp_col.scl[0],
					&data->bp_col.scl[1],
					&data->bp_col.scl[2]);
		}
		/* ne <x> <y> <z> <sx> <sy> <sz> */
		else if(strcmp(cmd_buf, "ne") == 0) {
			/* Read the data of the near-elipsoid-buffer */
			fscanf(fd, "%f %f %f %f %f %f",
					&data->ne_col.pos[0],
					&data->ne_col.pos[1],
					&data->ne_col.pos[2],
					&data->ne_col.scl[0],
					&data->ne_col.scl[1],
					&data->ne_col.scl[2]);
		}
		/* cv <x> <y> <z> */
		else if(strcmp(cmd_buf, "cv") == 0) {
			/* Increment the number of vertices */
			data->cm_vtx_c++;

			/* Allocate memory if necessary */
			if(data->cm_vtx_c == 1) {
				cm_vtx_num = 200;
				tmp = sizeof(float) * 3 * cm_vtx_num;

				if(!(data->cm_vtx_buf = calloc(1, tmp)))
					goto err_free_data;
			}
			else if(data->cm_vtx_c > cm_vtx_num) {
				cm_vtx_num *= 1.5;
				tmp = sizeof(float) * 3 * cm_vtx_num;

				if(!(p = realloc(data->cm_vtx_buf, tmp)))
					goto err_free_data;

				data->cm_vtx_buf = p;
			}

			/* Read the data of the new vertex */
			tmp = (data->cm_vtx_c - 1) * 3;
			fscanf(fd, "%f %f %f",
					&data->cm_vtx_buf[tmp + 0],
					&data->cm_vtx_buf[tmp + 1],
					&data->cm_vtx_buf[tmp + 2]);
		}
		/* ci <v1> <v2> <v3> */
		else if(strcmp(cmd_buf, "ci") == 0) {
			/* Increment the number of index-blocks */
			data->cm_idx_c++;

			/* Allocate memory if necessary */
			if(data->cm_idx_c == 1) {
				cm_idx_num = 200;
				tmp = sizeof(int) * 3 * cm_idx_num;

				if(!(data->cm_idx_buf = calloc(1, tmp)))
					goto err_free_data;
			}
			else if(data->cm_idx_c > cm_idx_num) {
				cm_idx_num *= 1.5;
				tmp = sizeof(int) * 3 * cm_idx_num;

				if(!(p = realloc(data->cm_idx_buf, tmp)))
					goto err_free_data;

				data->cm_idx_buf = p;
			}

			/* Read the data of the new vertex */
			tmp = (data->cm_idx_c - 1) * 3;
			fscanf(fd, "%d %d %d",
					&data->cm_idx_buf[tmp + 0],
					&data->cm_idx_buf[tmp + 1],
					&data->cm_idx_buf[tmp + 2]);

			data->cm_idx_buf[tmp + 0] -= 1;
			data->cm_idx_buf[tmp + 1] -= 1;
			data->cm_idx_buf[tmp + 2] -= 1;
		}
	}

	/* 
	 * Resize the arrays to make them fit their contents.
	 */

	/* vertex-buffer */
	if(data->vtx_c > 0) {
		tmp = data->vtx_c * 3 * sizeof(float);
		if(!(p = realloc(data->vtx_buf, tmp))) goto err_free_data;
		data->vtx_buf = p;
	}

	/* texture-buffer */
	if(data->tex_c > 0) {
		tmp = data->tex_c * 2 * sizeof(float);
		if(!(p = realloc(data->tex_buf, tmp))) goto err_free_data;
		data->tex_buf = p;
	}

	/* normal-buffer */
	if(data->nrm_c > 0) {
		tmp = data->nrm_c * 3 * sizeof(float);
		if(!(p = realloc(data->nrm_buf, tmp))) goto err_free_data;
		data->nrm_buf = p;
	}

	/* vertex-joint-buffer */
	if(data->vjnt_c > 0) {
		tmp = data->vjnt_c * 4 * sizeof(int);
		if(!(p = realloc(data->vjnt_buf, tmp))) goto err_free_data;
		data->vjnt_buf = p;
	}

	/* weight-buffer */
	if(data->wgt_c > 0) {
		tmp = data->wgt_c * 4 * sizeof(float);
		if(!(p = realloc(data->wgt_buf, tmp))) goto err_free_data;
		data->wgt_buf = p;
	}

	/* index-buffer */
	if(data->idx_c > 0) {
		tmp = (attr_m & AMO_M_RIG) ? 5 : 3;
		tmp = data->idx_c * 3 * tmp * sizeof(unsigned int);
		if(!(p = realloc(data->idx_buf, tmp))) goto err_free_data;
		data->idx_buf = p;
	}

	/* joint-list */
	if(data->jnt_c > 0) {
		tmp = data->jnt_c * sizeof(struct amo_joint);
		if(!(p = realloc(data->jnt_lst, tmp))) goto err_free_data;
		data->jnt_lst = p;
	}

	/* animation-list */
	if(data->ani_c > 0) {
		tmp = data->ani_c * sizeof(struct amo_anim);
		if(!(p = realloc(data->ani_lst, tmp))) goto err_free_data;
		data->ani_lst = p;

		for(i = 0; i < data->ani_c; i++) {
			struct amo_anim *ani = &data->ani_lst[i];
	
			tmp = ani->keyfr_c * sizeof(struct amo_keyfr);
			if(!(p = realloc(ani->keyfr_lst, tmp))) goto err_free_data;
			ani->keyfr_lst = p;
		}
	}

	/* collision-vertex-buffer */
	if(data->cm_vtx_c > 0) {
		tmp = data->cm_vtx_c * 3 * sizeof(float);
		if(!(p = realloc(data->cm_vtx_buf, tmp))) goto err_free_data;
		data->cm_vtx_buf = p;
	}

	/* collision-index-buffer */
	if(data->cm_idx_c > 0) {
		tmp = data->cm_idx_c * 3 * sizeof(int);
		if(!(p = realloc(data->cm_idx_buf, tmp))) goto err_free_data;
		data->cm_idx_buf = p;
	}

	/* Close the file */
	fclose(fd);

	/* Return the data */
	return data;

err_free_data:
	/* Free the allocated memory */
	amo_destroy(data);

err_close_file:
	fclose(fd);
	return NULL;
}


AMO_API void amo_destroy(struct amo_model *data)
{
	int i;
	int j;

	/* If data is NULL, just skip destroying */
	if(!data)
		return;

	free(data->vtx_buf);
	free(data->tex_buf);
	free(data->nrm_buf);

	if(data->jnt_c != 0) {
		free(data->vjnt_buf);
		free(data->wgt_buf);
		free(data->jnt_lst);

		for(i = 0; i < data->ani_c; i++) {
			for(j = 0; j < data->ani_lst[i].keyfr_c; j++) {
				free(data->ani_lst[i].keyfr_lst[j].joints);
				free(data->ani_lst[i].keyfr_lst[j].pos);
				free(data->ani_lst[i].keyfr_lst[j].rot);
			}

			free(data->ani_lst[i].keyfr_lst);
		}

		free(data->ani_lst);
	}

	free(data);
}


AMO_API int amo_getdata(struct amo_model *data, int *vtxnum, void **vtx,
		void **tex, void **nrm, void **jnt, void **wgt, int *idxnum,
		unsigned int **idx)
{
	int j;
	int k;
	int tmp;
	char same = 0;
	int num;
	char jnt_flg = 0;

	int vtx_num = 0;
	int vtx_i = 0;

	float *vtx_arr = NULL;
	float *tex_arr = NULL;
	float *nrm_arr = NULL;
	int *jnt_arr = NULL;
	float *wgt_arr = NULL;

	int *idx_conv = NULL;
	unsigned int *idx_arr = NULL;
	int idx_i = 0;
	int idx_bytes = 3;
	int idx_sz = 3 * sizeof(unsigned int);

	/* Return joints only if requested and possible */
	if(jnt != NULL && wgt != NULL && data->jnt_c != 0)
		jnt_flg = 1;

	/*
	 * Count the number of indices and allocate the necessary memory.
	 */

	tmp = data->idx_c * 3;

	if(!(idx_conv = malloc(tmp * sizeof(int))))
		return -1;

	if(!(idx_arr = malloc(tmp * sizeof(unsigned int))))
		goto err_free_arr;

	/*
	 * Bring order to the index-array.
	 */

	/* Get the format of the model */
	idx_bytes = (data->attr_m & AMO_M_RIG) ? 5 : 3;
	idx_sz = 3 * sizeof(unsigned int);

	num = data->idx_c * 3 * idx_bytes;
	for(j = 0; j < num; j += idx_bytes) {
		unsigned int cur[3], cmp[3];

		same = 0;
		memcpy(cur, &data->idx_buf[j], idx_sz);

		for(k = 0; k < idx_i; k++) {
			if(idx_conv[k] < 0)
				continue;
			
			memcpy(cmp, &data->idx_buf[idx_conv[k]], idx_sz);

			if(memcmp(cur, cmp, idx_sz) == 0) {
				same = 1;

				idx_conv[idx_i] = -1;
				idx_arr[idx_i] = idx_arr[k];
				idx_i++;
				break;
			}
		}

		if(!same) {
			idx_conv[idx_i] = j;
			idx_arr[idx_i] = vtx_num;

			idx_i++;
			vtx_num++;
		}
	}

	if(!(vtx_arr = malloc(vtx_num * 3 * sizeof(float))))
		goto err_free_arr;

	if(!(tex_arr = malloc(vtx_num * 2 * sizeof(float))))
		goto err_free_arr;

	if(!(nrm_arr = malloc(vtx_num * 3 * sizeof(float))))
		goto err_free_arr;

	if(jnt_flg) {
		if(!(jnt_arr = malloc(vtx_num * 4 * sizeof(int))))
			goto err_free_arr;

		if(!(wgt_arr = malloc(vtx_num * 4 * sizeof(float))))
			goto err_free_arr;
	}


	/* 
	 * Fill in vertex-data.
	 */
	for(j = 0; j < idx_i; j++) {
		if(idx_conv[j] < 0)
			continue;

		/* Copy vertex-position */
		tmp = data->idx_buf[idx_conv[j]];
		memcpy(&vtx_arr[vtx_i * 3], &data->vtx_buf[tmp * 3],
				3 * sizeof(float));

		/* Copy uv-coordinate */
		tmp = data->idx_buf[idx_conv[j] + 1];
		memcpy(&tex_arr[vtx_i * 2], &data->tex_buf[tmp * 2],
				2 * sizeof(float));

		/* Copy normal-vector */
		tmp = data->idx_buf[idx_conv[j] + 2];
		memcpy(&nrm_arr[vtx_i * 3], &data->nrm_buf[tmp * 3],
				3 * sizeof(float));

		if(jnt_arr) {
			/* Copy joint-indices */
			tmp = data->idx_buf[idx_conv[j] + 3];
			memcpy(&jnt_arr[vtx_i * 4], &data->vjnt_buf[tmp * 4],
				4 * sizeof(int));

			/* Copy joint-weights */
			tmp = data->idx_buf[idx_conv[j] + 4];
			memcpy(&wgt_arr[vtx_i * 4], &data->wgt_buf[tmp * 4],
				4 * sizeof(float));
		}

		vtx_i++;
	}

	/*
	 * Free the index-convert-array.
	 */
	free(idx_conv);

	/*
	 * Return all arrays.
	 */
	*vtxnum = vtx_i;
	*vtx = vtx_arr;
	*tex = tex_arr;
	*nrm = nrm_arr;
	*jnt = jnt_arr;
	*wgt = wgt_arr;

	*idxnum = idx_i;
	*idx = idx_arr;
	return 0;

err_free_arr:
	if(vtx_arr) free(vtx_arr);
	if(tex_arr) free(tex_arr);
	if(nrm_arr) free(nrm_arr);
	if(jnt_arr) free(jnt_arr);
	if(wgt_arr) free(wgt_arr);
	if(idx_conv) free(idx_conv);
	if(idx_arr) free(idx_arr);
	return -1;
}
