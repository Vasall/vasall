#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include "model.h"
#include "global.h"

#define DEGTORAD 3.14/180.0

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

	mdl = malloc(sizeof(struct model));
	if(mdl == NULL) {
		printf("Failed to allocate space for model.\n");
		return(NULL);
	}

	glGenVertexArrays(1, &mdl->vao);

	memset(mdl->bia, 0, 16 * sizeof(uint32_t));
	mdl->bia_num = 0;

	memset(mdl->bea, 0, 16 * sizeof(int8_t));
	mdl->bea_num = 0;

	mdl->vtx_len = 0;
	mdl->idx_len = 0;

	mdl->shader = shdBegin();
	if(mdl->shader == NULL) { 
		mdl->status = MESH_ERR_CREATING;
		return(NULL);
	}

	/* Set the status on  */
	mdl->status = MESH_OK;

	return(mdl);
}

/*
 * End the creating of a new model.
 * Do that by linking the shader.
 *
 * @mdl: Pointer to the model to finish
 *
 * Returns: The current status of the model
 */
int mdlFinish(struct model *mdl)
{
	/* Finish shader-program */
	glLinkProgram(mdl->shader->prog);

	if(mdl->status != 0) {
		mdl->status = MESH_ERR_FINISHING;
	}	

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
	uint32_t vbo, ibo;
	
	/* Bind vertex array object */
	glBindVertexArray(mdl->vao);

	/* ======= VERTICES ======== */
	/* Create vertex buffer object */
	glGenBuffers(1, &vbo);

	/* Copy our vertices array in a vertex buffer for OpenGL to use */
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vtxlen * VEC3_SIZE, 
			vtxbuf, GL_STATIC_DRAW);

	/* 1st attribute buffer : vertices */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	mdl->vtx_len = vtxlen;

	mdl->bia[mdl->bia_num] = vbo;
	mdl->bia_num++;

	/* ======== INDEX ========== */
	/* Create an element buffer object */
	glGenBuffers(1, &ibo);

	/* Copy our index array in a element buffer for OpenGL to use */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxlen * sizeof(unsigned int), 
			idxbuf, GL_STATIC_DRAW);

	mdl->idx_len = idxlen;

	mdl->bia[mdl->bia_num] = ibo;
	mdl->bia_num++;

	if(nrmflg) {
		uint32_t nbo;
		int i;
		Vec3 *normals;

		normals = calloc(vtxlen, VEC3_SIZE);

		for(i = 0; i < idxlen - 2; i += 3) {
			Vec3 v1, v2, v3, del1, del2, nrm;

			vecCpy(v1, vtxbuf[idxbuf[i]]);
			vecCpy(v2, vtxbuf[idxbuf[i + 1]]);
			vecCpy(v3, vtxbuf[idxbuf[i + 2]]);
			vecSub(v2, v1, del1);
			vecSub(v2, v3, del2);
			vecCross(del1, del2, nrm);
			vecNrm(nrm, nrm);

			vecCpy(normals[idxbuf[i]], nrm);
		}

		/* ======== NORMALS ======== */
		/* Create normals buffer object */
		glGenBuffers(1, &nbo);

		/* Copy our normals into a buffer for OpenGL to use */
		glBindBuffer(GL_ARRAY_BUFFER, nbo);
		glBufferData(GL_ARRAY_BUFFER, vtxlen * VEC3_SIZE, 
				normals, GL_STATIC_DRAW);

		/* 2st attribute buffer : normals */
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		mdl->bia[mdl->bia_num] = vbo;
		mdl->bia_num++;
	}

	/* Unbind vao */
	glBindVertexArray(0);
}

/* 
 * Attach a new buffer to the model and 
 * if specified enable the buffer.
 *
 * @mdl: Pointer to the model
 * @buf: The buffer to insert
 * @elsize: The size of one element in the buffer
 * @num: The number of elements in the buffer
 * @en: Should the buffer be bound and to what index
 * @sz: The number of float values per element(or 0 when en=0)
 */
void mdlAddBAO(struct model *mdl, void *buf, int elsize, int num, 
		uint8_t bindflg, uint8_t bindsz)
{
	uint32_t bao;

	/* Bind vertex-array-object */
	glBindVertexArray(mdl->vao);

	/* Create a buffer-array-object */
	glGenBuffers(1, &bao);

	/* Copy our color attay in a different one for OpenGL to use */
	glBindBuffer(GL_ARRAY_BUFFER, bao);
	glBufferData(GL_ARRAY_BUFFER, num * elsize, buf, GL_STATIC_DRAW);

	if(bindflg) {
		glVertexAttribPointer(bindflg, bindsz, GL_FLOAT, 
				GL_FALSE, 0, NULL);
	}

	/* Add the bao to the buffer-index-array */
	mdl->bia[mdl->bia_num] = bao;
	mdl->bia_num++;

	/* Unbind vao */
	glBindVertexArray(0);
}

/* 
 * Calculate the normal-vectors 
 * for the model and create a new
 * buffer-object.
 *
 * @mdl: Pointer to the model to 
 * 	calculate the normals for 
 */
void mdlCalcNormals(struct model *mdl)
{
	if(mdl) {}
}


/*
 * Render a model.
 *
 * @mdl: Pointer to the model to render
 * @mat: The matrix that should be used for transformation
 */
void mdlRender(struct model *mdl, Mat4 mat)
{
	int err, model, view, proj;

	Mat4 mod, vie, pro;

	mat4Cpy(mod, mat);
	camGetView(vie);
	camGetProj(pro);

	glBindVertexArray(mdl->vao);
	glUseProgram(mdl->shader->prog);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

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

	glDrawElements(GL_TRIANGLES, mdl->idx_len, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

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
	glBindAttribLocation(mdl->shader->prog, 0, "vtxPos");
	glBindAttribLocation(mdl->shader->prog, 1, "vtxNrm");
	glBindAttribLocation(mdl->shader->prog, 2, "vtxCol");
	if(mdlFinish(mdl) < 0) return(NULL);

	return(mdl);
}
