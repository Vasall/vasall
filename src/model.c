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
Model *mdlBegin(void)
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

	mdl->trans = mat4Idt();

	mdl->rot = mat4Idt();

	mdl->scl = mat4Idt();

	mdl->status = 0;

	mdl->shader = shdBegin();
	if(mdl->shader == NULL) {
		printf("Failed to create shader.\n");
		return(NULL);
	}

	return(mdl);
}

/*
 * End the creating of a new model.
 * Do that by linking the shader.
 *
 * @mdl: Pointer to the model to finish
 *
 * Returns: Either 0 on success or
 * 	-1 if an error occurred
 */
int mdlEnd(Model *mdl)
{
	/* Finish shader-program */
	glLinkProgram(mdl->shader->prog);

	if(mdl->status != 0) {
		printf("Error: %x\n", mdl->status);
		return(-1);
	}

	return(0);
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
 */
void mdlLoadVtx(Model *mdl, Vertex *vtxbuf, int vtxlen, 
		uint32_t *idxbuf, int idxlen)
{
	uint32_t vbo, ibo;

	/* Bind vertex array object */
	glBindVertexArray(mdl->vao);

	/* ========= MESH ========== */
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

	/* Unbind vao */
	glBindVertexArray(0);
}

void mdlAttachColBuf(Model *mdl, ColorRGB *colbuf, int collen)
{
	uint32_t cbo;
	
	/* Bind vertex array object */
	glBindVertexArray(mdl->vao);

	/* ======== COLORS ========= */
	/* Create a color buffer object */
	glGenBuffers(1, &cbo);

	/* Copy our color attay in a different one for OpenGL to use */
	glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glBufferData(GL_ARRAY_BUFFER, collen * sizeof(ColorRGB), 
			colbuf, GL_STATIC_DRAW);

	/* 2nd attribute buffer : colors */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	/* Unbind vao */
	glBindVertexArray(0);
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

	mod = mdlGetMdlMat(mdl);
	vie = camGetView(core->camera);
	pro = camGetProj(core->camera);

	glBindVertexArray(mdl->vao);
	glUseProgram(mdl->shader->prog);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

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
Mat4 mdlGetMdlMat(Model *mdl)
{
	Mat4 ret = mat4Idt();
	/*mat4Mult(ret, mdl->scl);*/
	ret = mdl->rot;
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
void mdlSetRot(Model *mdl, float x, float y, float z)
{
	x *= DEGTORAD;
	y *= DEGTORAD;
	z *= DEGTORAD;

	memset(mdl->rot, 0, 16 * sizeof(float));
/*
	mdl->rot[0x0] = 1;
	mdl->rot[0x5] = cos(x);
	mdl->rot[0x6] = sin(x);
	mdl->rot[0x9] = -sin(x);
	mdl->rot[0xa] = cos(x);
*/
	mdl->rot[0x0] = cos(y);
	mdl->rot[0x2] = -sin(y);
	mdl->rot[0x8] = sin(y);
	mdl->rot[0xa] = cos(y);
/*
	mdl->rot[0x0] *= cos(z);
	mdl->rot[0x1] = sin(z);
	mdl->rot[0x4] = -sin(z);
	mdl->rot[0x5] *= cos(z);
*/
	mdl->rot[0x5] = 1.0;
	mdl->rot[0xf] = 1.0;
}
