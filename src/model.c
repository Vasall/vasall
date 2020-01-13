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
 * @pos: The position-reference of the model
 * @rot: The rotation-reference of the model
 *
 * Returns: Either a pointer to the new
 * 	model of NULL
 */
Model *mdlCreate(Vec3 *pos, Vec3 *rot)
{
	Model *mdl = NULL;

	mdl = malloc(sizeof(Model));
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

	mdl->trans_mat = mat4Idt();

	mdl->rot_mat = mat4Idt();

	mdl->scl_mat = mat4Idt();

	/* Copy both the position- and the rotation-reference */
	mdl->pos = pos;
	mdl->rot = rot;

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
int mdlFinish(Model *mdl)
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
void mdlSetMesh(Model *mdl, Vertex *vtxbuf, int vtxlen, 
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
	glBufferData(GL_ARRAY_BUFFER, vtxlen * sizeof(Vertex), 
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

		normals = calloc(vtxlen, sizeof(Vec3));

		for(i = 0; i < idxlen - 2; i += 3) {
			Vec3 v1 = vtxbuf[idxbuf[i]];
			Vec3 v2 = vtxbuf[idxbuf[i + 1]];
			Vec3 v3 = vtxbuf[idxbuf[i + 2]];
			Vec3 del1 = vecSubRet(v2, v1);
			Vec3 del2 = vecSubRet(v2, v3);
			Vec3 nrm = vecNrmRet(vecCross(del1, del2));

			normals[idxbuf[i]] = nrm;
		}

		/* ======== NORMALS ======== */
		/* Create normals buffer object */
		glGenBuffers(1, &nbo);

		/* Copy our normals into a buffer for OpenGL to use */
		glBindBuffer(GL_ARRAY_BUFFER, nbo);
		glBufferData(GL_ARRAY_BUFFER, vtxlen * sizeof(Vec3), 
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
void mdlAddBAO(Model *mdl, void *buf, int elsize, int num, 
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
void mdlCalcNormals(Model *mdl)
{
	if(mdl) {}
}


/*
 * Render a model.
 *
 * @mdl: Pointer to the model to render
 */
void mdlRender(Model *mdl)
{
	int err, model, view, proj;

	Mat4 mod, vie, pro;

	mod = mdlGetMatrix(mdl);
	vie = camGetView(core->camera);
	pro = camGetProj(core->camera);

	glBindVertexArray(mdl->vao);
	glUseProgram(mdl->shader->prog);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	model = glGetUniformLocation(mdl->shader->prog, "model");
	if(model == -1) {printf("model!!\n"); exit(1);}
	view = glGetUniformLocation(mdl->shader->prog, "view");
	if(view == -1) {printf("view!!\n"); exit(1);}
	proj = glGetUniformLocation(mdl->shader->prog, "proj");
	if(proj == -1) {printf("proj!!\n"); exit(1);}

	glUniformMatrix4fv(model, 1, GL_FALSE, mod);
	glUniformMatrix4fv(view, 1, GL_FALSE, vie);
	glUniformMatrix4fv(proj, 1, GL_FALSE, pro);

	err = glGetError();
	if(err != 0) printf("Error: %x\n", err);

	glDrawElements(GL_TRIANGLES, mdl->idx_len, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

/*
 * Get the modelmatrix for rendering.
 * This function will combine all 3
 * model-matrices and return the result
 * as a new matrix.
 *
 * @mdl: Pointer to the model
 *
 * Returns: The resulting matrix,
 * 	note that this function will
 * 	return garbage if an error
 * 	occurred
 */
Mat4 mdlGetMatrix(Model *mdl)
{
	Mat4 ret = mat4Idt();
	/*mat4Mult(ret, mdl->scl);*/
	ret = mdl->rot_mat;
	/*mat4Mult(ret, mdl->trans);*/
	return(ret);
}

/*
 * TODO: Add the other two rotation
 * axis.
 * 	
 * Set the rotation of a model.
 *
 * @mdl: The model to modify
 * @x: The x-axis-rotation
 * @y: The y-axis-rotation
 * @z: The z-axis-rotation
 */
void mdlSetRot(Model *mdl)
{
	mdl->pos->x *= DEGTORAD;
	mdl->pos->y *= DEGTORAD;
	mdl->pos->z *= DEGTORAD;

	memset(mdl->rot_mat, 0, 16 * sizeof(float));

	mdl->rot_mat[0x0] = cos(mdl->pos->y);
	mdl->rot_mat[0x2] = -sin(mdl->pos->y);
	mdl->rot_mat[0x8] = sin(mdl->pos->y);
	mdl->rot_mat[0xa] = cos(mdl->pos->y);

	mdl->rot_mat[0x5] = 1.0;
	mdl->rot_mat[0xf] = 1.0;
}
