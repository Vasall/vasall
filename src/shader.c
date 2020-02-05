#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "global.h"
#include "XSDL/xsdl.h"


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
	struct shader *shd = NULL;
	int success;
	char *vtxsrc = NULL, *frgsrc = NULL, infoLog[512];

	if(vtx_shd == NULL || frg_shd == NULL) return(-1);

	shd = malloc(sizeof(struct shader));
	if(shd == NULL) return(-1);

	shd->prog = glCreateProgram();

	/* Load vertex-shader */	
	vtxsrc = filetobuf(vtx_shd);
	if(vtxsrc == NULL) goto failed;

	shd->vshdr = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shd->vshdr, 1, (const GLchar **)&vtxsrc, NULL);
	glCompileShader(shd->vshdr);

	glGetShaderiv(shd->vshdr, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(shd->vshdr, 512, NULL, infoLog);
		printf("Failed to compile shader %s: %s\n", vtx_shd, infoLog);
		goto failed;
	}
	
	glAttachShader(shd->prog, shd->vshdr);

	/* Load fragment-shader */
	frgsrc = filetobuf(frg_shd);
	if(frgsrc == NULL) goto failed;

	shd->fshdr = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shd->fshdr, 1, (const GLchar **)&frgsrc, NULL);
	glCompileShader(shd->fshdr);

	glGetShaderiv(shd->fshdr, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(shd->fshdr, 512, NULL, infoLog);
		printf("Failed to compile shader %s: %s\n", frg_shd, infoLog);
		goto failed;
	}
		
	glAttachShader(shd->prog, shd->fshdr);

	glBindAttribLocation(shd->prog, 0, "vtxPos");
	glBindAttribLocation(shd->prog, 1, "vtxNrm");
	glBindAttribLocation(shd->prog, 2, "vtxUV");
	
	glLinkProgram(shd->prog);

	htSet(shader_table, key, (const uint8_t *)shd, sizeof(struct shader));
	

	free(vtxsrc);
	free(frgsrc);

	return(0);

failed:
	free(vtxsrc);
	free(frgsrc);
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

/* Delete a shader an remove it from the shader-table */
void shdDel(char *key);
