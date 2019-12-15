#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>

#include "shader.h"
#include "global.h"
#include "../enud/enud.h"

char* filetobuf(char *file);

/* A simple function that will read a file into an allocated char pointer buffer */
char* filetobuf(char *file)
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
 * Create a new shader program, and load both the
 * vertex-shader and fragment-shader.
 *
 * @vtx_shd: The relative path to the vertex-shader-file
 * @frg_shd: The rlative path to the fragment-shader-file
 *
 * Returns: Either a pointer to the created shader-program
 * 	or NULL if an error occurred
 */
Shader *shdCreate(char *vtx_shd, char *frg_shd)
{
	int success;
	char path[512], *vtxsrc, *frgsrc, infoLog[512];
	Shader *shd;

	shd = calloc(1, sizeof(Shader));
	if(shd == NULL) {
		return(NULL);
	}

	shd->id = glCreateProgram();

	if(vtx_shd != NULL) {
		ENUD_CombinePath(path, core->bindir, vtx_shd);
		vtxsrc = filetobuf(path);
		if(vtxsrc != NULL) {
			shd->vshdr = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(shd->vshdr, 1, (const GLchar**)&vtxsrc, NULL);
			glCompileShader(shd->vshdr);

			glGetShaderiv(shd->vshdr, GL_COMPILE_STATUS, &success);
			if(!success) {
				glGetShaderInfoLog(shd->vshdr, 512, NULL, infoLog);
				printf("Failed to compile shader %s: %s\n",
						path, infoLog);
				exit(1);
			}
			glAttachShader(shd->id, shd->vshdr);
			printf("Added vertex-shader\n");
		}
		else {
			printf("Failed to load %s\n", path);
			exit(1);
		}
		free(vtxsrc);
	}

	if(frg_shd != NULL) {
		ENUD_CombinePath(path, core->bindir, frg_shd);
		frgsrc = filetobuf(path);
		if(frgsrc != NULL) {
			shd->fshdr = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(shd->fshdr, 1, (const GLchar**)&frgsrc, NULL);
			glCompileShader(shd->fshdr);

			glGetShaderiv(shd->fshdr, GL_COMPILE_STATUS, &success);
			if(!success) {
				glGetShaderInfoLog(shd->fshdr, 512, NULL, infoLog);
				printf("Failed to compile shader %s: %s\n", path, infoLog);
				exit(1);
			}
			glAttachShader(shd->id, shd->fshdr);
			printf("Added fragment-shader\n");

		}
		else {
			printf("Failed to load %s\n", path);
			exit(1);
		}
	}

	return(shd);
}

