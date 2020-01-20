#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include "model.h"
#include "global.h"

/* Redefine external variables */
struct model **model_cache = NULL;

/* 
 * Start the creation of a new model. 
 * First push a new model struct into 
 * the model-cache and mark the new 
 * struct as the currently active one.
 *
 * Returns: Either a pointer to the new
 * 	model of NULL
 */
struct model *mdlCreate(void)
{
	struct model *mdl = NULL;
	
	/* Allocate memory for the model-struct */
	mdl = malloc(sizeof(struct model));
	if(mdl == NULL) goto failed;

	/* Create a vao */
	glGenVertexArrays(1, &mdl->vao);

	/* Initialize the stack for the baos */
	mdl->bao = stcCreate(sizeof(struct bao_entry *));
	if(mdl->bao == NULL) goto failed;

	/*  Create the shader */
	mdl->shader = shdBegin();
	if(mdl->shader == NULL) goto failed;

	/* Set the status to OK */
	mdl->status = MDL_OK;

	return(mdl);

failed:
	/* Set the status to FAILED */
	mdl->status = MDL_ERR_CREATING;
	return(NULL);
}

/*
 * End the creating of a new model.
 * Do that by linking the shader.
 *
 * @mdl: Pointer to the model to finish
 *
 * Returns: The current final status of the model
 */
int mdlFinish(struct model *mdl)
{
	/* Finish shader-program */
	glLinkProgram(mdl->shader->prog);

	return(mdl->status);
}

/*
 * Insert both the vertices and the
 * indices to the model.
 *
 * @mdl: Pointer to the model
 * @vtxbuf: The vertex-buffer
 * @vtxlen: The length of the vertex-buffer
 * @idxbuf: The index-buffer
 * @idxlen: The length of the index-buffer
 * @nrmflg: This flag indicates, if the normales should be calculated
 */
void mdlSetMesh(struct model *mdl, Vec3 *vtxbuf, int vtxlen, 
		uint32_t *idxbuf, int idxlen, uint8_t nrmflg)
{
	int i;
	Vec3 *nrmbuf;
	
	if(mdl->status != MDL_OK) return;

	mdlAddBAO(mdl, 0, vtxbuf, VEC3_SIZE, vtxlen, 0, 3, 0, "vtxPos");
	mdlAddBAO(mdl, 1, idxbuf, sizeof(uint32_t), idxlen, -1, 0, 0, NULL);

	nrmbuf = calloc(vtxlen, VEC3_SIZE);

	for(i = 0; i < idxlen - 2; i += 3) {
		Vec3 v1, v2, v3, del1, del2, nrm;

		vecCpy(v1, vtxbuf[idxbuf[i]]);
		vecCpy(v2, vtxbuf[idxbuf[i + 1]]);
		vecCpy(v3, vtxbuf[idxbuf[i + 2]]);
		vecSub(v2, v1, del1);
		vecSub(v2, v3, del2);
		vecCross(del1, del2, nrm);
		vecNrm(nrm, nrm);

		vecCpy(nrmbuf[idxbuf[i]], nrm);
	}
	
	if(nrmflg) {
		mdlAddBAO(mdl, 0, nrmbuf, VEC3_SIZE, vtxlen, 1, 3, 0, "vtxNrm");
	}
}

/* 
 * Attach a new buffer to the model and 
 * if specified enable the buffer.
 *
 * @mdl: Pointer to the model
 * @atype: The type of array to add
 * 	- 0: GL_ARRAY_BUFFER
 * 	- 1: GL_ELEMENT_ARRAY_BUFFER
 * @buf: The buffer to insert
 * @size: The size of one element in the buffer
 * @num: The number of elements in the buffer
 * @bindex: Should the buffer be bound and to what index
 * @bsize: The number of float values per element
 * @btype: The type of data used
 * 	- 0: GL_FLOAT
 * @bname: The name of the variable to bind the bao to
 */
void mdlAddBAO(struct model *mdl, uint8_t atype, void *buf, int size, int num, 
		int8_t bindex, uint8_t bsize, uint8_t btype, char *bname)
{
	uint32_t bao, tmp;
	struct bao_entry *bao_stc = NULL;
	GLenum target;
	
	/* Allocate memory for the bao-struct */
	if((bao_stc = malloc(sizeof(struct bao_entry))) == NULL) goto failed;

	/* Allocate memory for the data-buffer */
	bao_stc->ele_size = size;
	bao_stc->ele_num = num;
	tmp = bao_stc->ele_num * bao_stc->ele_size;
	if((bao_stc->buf = malloc(tmp)) == NULL) goto failed;

	/* Copy the data into the buffer */
	memcpy(bao_stc->buf, buf, tmp);

	/* Bind vertex-array-object */
	glBindVertexArray(mdl->vao);

	/* Create a buffer-array-object */
	glGenBuffers(1, &bao);
	bao_stc->index = bao;

	/* Determit which array-type to use */
	target = (!atype) ? (GL_ARRAY_BUFFER) : (GL_ELEMENT_ARRAY_BUFFER);

	/* Copy the array into the bao-buffer */
	glBindBuffer(target, bao);
	glBufferData(target, tmp, bao_stc->buf, GL_STATIC_DRAW);

	/* If the bao should be bound to an index */
	if(bindex >= 0) {
		GLenum data_type = (!btype) ? (GL_FLOAT) : (0);

		bao_stc->attr_ptr = bindex;
		strcpy(bao_stc->attr_name, bname);

		glVertexAttribPointer(bindex, bsize, data_type, 
				GL_FALSE, 0, NULL);

		shdBindAttr(mdl->shader, bindex, bname);
	}
	else {
		bao_stc->attr_ptr = -1;
	}

	/* Push the bao into the bao-stack */
	stcPush(mdl->bao, &bao_stc);

	/* Unbind vao */
	glBindVertexArray(0);

	return;

failed:
	free(bao_stc->buf);
	free(bao_stc);

	/* Set the status to failed */
	mdl->status = MDL_ERR_ADD_BAO;
}

/*
 * Render a model.
 *
 * @mdl: Pointer to the model to render
 * @mat: The matrix that should be used for transformation
 */
void mdlRender(struct model *mdl, Mat4 mat)
{
	int i, attr, err, model, view, proj;
	int idxnum = 0;
	Mat4 mod, vie, pro;

	if(mdl->status != MDL_OK) return;

	mat4Cpy(mod, mat);
	camGetView(vie);
	camGetProj(pro);

	glBindVertexArray(mdl->vao);
	glUseProgram(mdl->shader->prog);
	
	for(i = 0; i < mdl->bao->num; i++) {
		attr = 	((struct bao_entry **)mdl->bao->buf)[i]->attr_ptr;
		if(attr != -1) glEnableVertexAttribArray(attr);
	}

	model = glGetUniformLocation(mdl->shader->prog, "model");
	if(model == -1) { return; }
	view = glGetUniformLocation(mdl->shader->prog, "view");
	if(view == -1) { return; }
	proj = glGetUniformLocation(mdl->shader->prog, "proj");
	if(proj == -1) { return; }

	glUniformMatrix4fv(model, 1, GL_FALSE, mod);
	glUniformMatrix4fv(view, 1, GL_FALSE, vie);
	glUniformMatrix4fv(proj, 1, GL_FALSE, pro);

	err = glGetError();
	if(err != 0) printf("Error: %x\n", err);

	idxnum = ((struct bao_entry **)mdl->bao->buf)[1]->ele_num;
	glDrawElements(GL_TRIANGLES, idxnum, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

#ifdef DEBUG
/*
 * Create a red-cube as a model (used for dev).
 *
 * @pos: A position-reference
 * @rot: A rotation-reference
 *
 * Returns: Either a model containing a red cube
 * 	or NULL if an error occurred
 */
struct model *mdlRedCube(void)
{
	struct model *mdl;
	Vec3 *vtx;
	uint32_t *idx;
	ColorRGB *col;
	int i;

	vtx = malloc(8 * VEC3_SIZE);
	idx = malloc(36 * sizeof(uint32_t));
	col = malloc(8 * sizeof(ColorRGB));

	vecSet(vtx[0], -1.0, -1.0,  1.0);
	vecSet(vtx[1],  1.0, -1.0,  1.0);
	vecSet(vtx[2],  1.0,  1.0,  1.0); 
	vecSet(vtx[3], -1.0,  1.0,  1.0);
	vecSet(vtx[4], -1.0, -1.0, -1.0);
	vecSet(vtx[5],  1.0, -1.0, -1.0);
	vecSet(vtx[6],  1.0,  1.0, -1.0);
	vecSet(vtx[7], -1.0,  1.0, -1.0);

	idx[0] = 0;   idx[1] = 1;  idx[2] = 2;
	idx[3] = 2;   idx[4] = 3;  idx[5] = 0;
	idx[6] = 1;   idx[7] = 5;  idx[8] = 6;
	idx[9] = 6;  idx[10] = 2; idx[11] = 1;
	idx[12] = 7; idx[13] = 6; idx[14] = 5;
	idx[15] = 5; idx[16] = 4; idx[17] = 7;
	idx[18] = 4; idx[19] = 0; idx[20] = 3;
	idx[21] = 3; idx[22] = 7; idx[23] = 4;
	idx[24] = 4; idx[25] = 5; idx[26] = 1;
	idx[27] = 1; idx[28] = 0; idx[29] = 4;
	idx[30] = 3; idx[31] = 2; idx[32] = 6;
	idx[33] = 6; idx[34] = 7; idx[35] = 3;

	for(i = 0; i < 8; i++) {
		col[i].r = 1.0;
		col[i].g = 0.0;
		col[i].b = 0.0;
	}	

	if((mdl = mdlCreate()) == NULL) return(NULL);
	mdlSetMesh(mdl, vtx, 8, idx, 36, 1);
	mdlAddBAO(mdl, col, sizeof(ColorRGB), 8, 2, 3);
	shdAttachVtx(mdl->shader, "../res/shaders/flat.vert");
	shdAttachFrg(mdl->shader, "../res/shaders/flat.frag");
	shdBindAttr(mdl->shader, 0, "vtxPos");
	shdBindAttr(mdl->shader, 1, "vtxNrm");
	shdBindAttr(mdl->shader, 2, "vtxCol");
	if(mdlFinish(mdl) < 0) return(NULL);

	return(mdl);
}
#endif
