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
struct shader **shaders = NULL;


/* A simple function that will read a file into an allocated char pointer buffer */
static char *filetobuf(char *file)
{
	FILE *fptr;
	long length;
	char *buf;

	if(!(fptr = fopen(file, "rb")))
		return NULL;
	
	fseek(fptr, 0, SEEK_END);
	length = ftell(fptr);
	buf = malloc(length + 1);
	fseek(fptr, 0, SEEK_SET);
	fread(buf, length, 1, fptr);
	fclose(fptr);
	buf[length] = 0;

	return buf;
}

static short shd_get_slot(void)
{
	short i;

	for(i = 0; i < SHD_SLOTS; i++) {
		if(!shaders[i])
			return i;
	}

	return -1;
}

int shd_init(void)
{
	int i;

	if(!(shaders = malloc(sizeof(struct shader *) * SHD_SLOTS)))
		return -1;

	for(i = 0; i < SHD_SLOTS; i++)
		shaders[i] = NULL;

	return 0;
}

void shd_close(void)
{
	int i;
	struct shader *shd;

	for(i = 0; i < SHD_SLOTS; i++) {
		if(!(shd = shaders[i]))
			continue;
	
		/* Destroy the shader-program */
		glDeleteProgram(shd->prog);	
	}

	free(shaders);
}

short shd_set(char *name, char *vtx_shd, char *frg_shd)
{
	short slot;
	int success;
	int shd_sz = sizeof(struct shader);
	struct shader *shd = NULL;
	uint32_t fshd = 0, vshd = 0;
	char *vtxsrc = NULL, *frgsrc = NULL, infoLog[512];

	if((slot = shd_get_slot()) < 0)
		return -1;

	if(!vtx_shd || !frg_shd)
		return -1;

	/* Allocate the necessary memory for the struct */
	if(!(shd = malloc(shd_sz)))
		return -1;

	/* Initialize the shader-values */
	strcpy(shd->name, name);
	shd->prog = 0;
	shd->status = 0;

	/* Create a new shader-program */
	if(!(shd->prog = glCreateProgram()))
		goto err_cleanup;

	/* Load vertex-shader and attach the vertex-shader */
	if(!(vtxsrc = filetobuf(vtx_shd)))
		goto err_cleanup;

	/* Create and initialize the vertex-shader */
	vshd = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshd, 1, (const GLchar **)&vtxsrc, NULL);
	glCompileShader(vshd);

	glGetShaderiv(vshd, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vshd, 512, NULL, infoLog);
		printf("\nFailed to compile shader %s: %s\n", vtx_shd, infoLog);
		goto err_cleanup;
	}

	glAttachShader(shd->prog, vshd);

	/* Load and attach the fragment-shader */
	if(!(frgsrc = filetobuf(frg_shd)))
		goto err_cleanup;

	/* Create and initialize the fragment-shader */
	fshd = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshd, 1, (const GLchar **)&frgsrc, NULL);
	glCompileShader(fshd);

	glGetShaderiv(fshd, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fshd, 512, NULL, infoLog);
		printf("\nFailed to compile shader %s: %s\n", frg_shd, infoLog);
		goto err_cleanup;
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

	shaders[slot] = shd;
	free(vtxsrc);
	free(frgsrc);
	return slot;

err_cleanup:
	/* Detach and destroy the shaders */
	if(vshd) {
		glDetachShader(shd->prog, vshd);
		glDeleteShader(vshd);
	}
	if(fshd) {
		glDetachShader(shd->prog, fshd);
		glDeleteShader(fshd);
	}

	/* Destroy the shader-program */
	if(shd->prog)
		glDeleteProgram(shd->prog);

	/* Free the code-buffers */
	free(vtxsrc);
	free(frgsrc);

	/* Destroy the shader-struct */
	free(shd);
	return -1;
}

short shd_get(char *name)
{
	int i;

	for(i = 0; i < SHD_SLOTS; i++) {
		if(!shaders[i])
			continue;

		if(!strcmp(shaders[i]->name, name))
			return i;
	}

	return -1;
}

void shd_del(short slot)
{
	struct shader *shd;

	if(slot < 0 || slot >= SHD_SLOTS)
		return;

	if(!(shd = shaders[slot]))
		return;

	/* Destroy the shader-program */
	if(shd->prog)
		glDeleteProgram(shd->prog);

	free(shd);
	shaders[slot] = NULL;
}
