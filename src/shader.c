#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


/* A list containing all active shaders */
struct ht_t *shader_table = NULL;


/* A simple function that will read a file into an allocated char pointer buffer */
static char* filetobuf(char *file)
{
	FILE *fptr;
	long length;
	char *buf;

	fptr = fopen(file, "rb");
	if (!fptr) {
		return NULL;
	}
	fseek(fptr, 0, SEEK_END);
	length = ftell(fptr);
	buf = malloc(length + 1);
	fseek(fptr, 0, SEEK_SET);
	fread(buf, length, 1, fptr);
	fclose(fptr);
	buf[length] = 0;

	return (buf);
}

/*
 * Initialize the shader-table and allocate the
 * necessary memory.
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int shdInit(void)
{
	shader_table = htCreate(SHD_SLOTS);
	if(shader_table == NULL) return(-1);

	return(0);
}

/*
 * Destroy the shader-table and free the allocated
 * memory.
 */
void shdClose(void)
{
	int i;
	struct ht_entry *ptr;
	struct shader *shd;

	for(i = 0; i < tex_table->size; i++) {
		ptr = shader_table->entries[i];

		while(ptr != NULL) {
			shd = (struct shader *)ptr->buf;

			/* Destroy the shader-program */
			glDeleteProgram(shd->prog);

			ptr = ptr->next;
		}
	}

	htDestroy(shader_table);
}

/*
 * Create a new shader and load both the vertex- and
 * fragment-shader-files. The compile the code and
 * push the finished shader-program into the table.
 */
int shdSet(char *key, char *vtx_shd, char *frg_shd)
{
	int success;
	struct shader *shd = NULL;
	uint32_t fshd = 0, vshd = 0;
	char *vtxsrc = NULL, *frgsrc = NULL, infoLog[512];

	if(vtx_shd == NULL || frg_shd == NULL) return(-1);

	/* Allocate the necessary memory for the struct */
	shd = malloc(sizeof(struct shader));
	if(shd == NULL) return(-1);

	/* Initialize the shader-values */
	shd->prog = 0;
	shd->status = 0;

	/* Create a new shader-program */
	shd->prog = glCreateProgram();
	if(shd->prog == 0) goto failed;

	/* Load vertex-shader and attach the vertex-shader */
	vtxsrc = filetobuf(vtx_shd);
	if(vtxsrc == NULL) goto failed;

	/* Create and initialize the vertex-shader */
	vshd = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshd, 1, (const GLchar **)&vtxsrc, NULL);
	glCompileShader(vshd);

	glGetShaderiv(vshd, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vshd, 512, NULL, infoLog);
		printf("\nFailed to compile shader %s: %s\n", vtx_shd, infoLog);
		goto failed;
	}

	glAttachShader(shd->prog, vshd);

	/* Load and attach the fragment-shader */
	frgsrc = filetobuf(frg_shd);
	if(frgsrc == NULL) goto failed;

	/* Create and initialize the fragment-shader */
	fshd = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshd, 1, (const GLchar **)&frgsrc, NULL);
	glCompileShader(fshd);

	glGetShaderiv(fshd, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fshd, 512, NULL, infoLog);
		printf("\nFailed to compile shader %s: %s\n", frg_shd, infoLog);
		goto failed;
	}

	glAttachShader(shd->prog, fshd);

	/* Bind the vertex-attributes */
	glBindAttribLocation(shd->prog, 0, "vtxPos");
	glBindAttribLocation(shd->prog, 1, "vtxNrm");
	glBindAttribLocation(shd->prog, 2, "vtxCol");

	/* Link the shader-program */
	glLinkProgram(shd->prog);

	/* Detach and destroy shaders */
	glDetachShader(shd->prog, vshd);
	glDeleteShader(vshd);
	vshd = 0;

	glDetachShader(shd->prog, fshd);
	glDeleteShader(fshd);
	fshd = 0;

	/* Insert the shader into the tabel */
	if(htSet(shader_table, key, (const uint8_t *)shd,
			sizeof(struct shader)) < 0) {
		goto failed;
	}

	/* Free the code-buffers */
	free(vtxsrc);
	free(frgsrc);

	return(0);

failed:
	/* Detach and destroy the shaders */
	if(vshd != 0) {
		glDetachShader(shd->prog, vshd);
		glDeleteShader(vshd);
	}
	if(fshd != 0) {
		glDetachShader(shd->prog, fshd);
		glDeleteShader(fshd);
	}

	/* Destroy the shader-program */
	if(shd->prog != 0) {
		glDeleteProgram(shd->prog);
	}

	/* Free the code-buffers */
	free(vtxsrc);
	free(frgsrc);

	/* Destroy the shader-struct */
	free(shd);

	return(-1);
}

/* Get a shader from the shader-table */
struct shader *shdGet(char *key)
{
	struct shader *ptr;

	if(htGet(shader_table, key, (uint8_t **)&ptr, NULL) < 0) {
		return(NULL);
	}

	return(ptr);
}

/*
 * Unbind everything from OpenGL, destroy the shader
 * and remove it from the shader-table.
 *
 * @key: The key of the shader to destroy
 */
void shdRemv(char *key)
{
	struct shader *shd;

	shd = shdGet(key);
	if(shd == NULL) return;

	/* Destroy the shader-program */
	if(shd->prog != 0) {
		glDeleteProgram(shd->prog);
	}

	htDel(shader_table, key);
}
