#include "shader.h"
#include "filesystem.h"

#include <stdlib.h>


struct shader_wrapper shaders;


static short shd_get_slot(void)
{
	short i;

	for(i = 0; i < SHD_SLOTS; i++) {
		if(shaders.mask[i] == 0)
			return i;
	}

	return -1;
}

static int shd_check_slot(short slot)
{
	if(slot < 0 || slot >= SHD_SLOTS)
		return 1;

	return 0;
}

extern int shd_init(void)
{
	int i;

	for(i = 0; i < SHD_SLOTS; i++)
		shaders.mask[i] = 0;

	return 0;
}

extern void shd_close(void)
{
	int i;

	for(i = 0; i < SHD_SLOTS; i++) {
		if(shaders.mask[i] == 0)
			continue;
	
		glDeleteProgram(shaders.prog[i]);	
	}
}

extern short shd_set(char *name, char *vs, char *fs)
{
	short slot;
	int success;
	uint32_t fshd = 0;
	uint32_t vshd = 0;
	char *vtx_src = NULL;
	char *frg_src = NULL;

	if((slot = shd_get_slot()) < 0)
		return -1;

	if(!vs || !fs)
		return -1;

	if((shaders.prog[slot] = glCreateProgram()) == 0)
		goto err_cleanup;

	/* Load vertex-shader and attach the vertex-shader */
	if(fs_load_file(vs, (uint8_t **)&vtx_src, NULL) < 0)
		goto err_cleanup;

	/* Create and initialize the vertex-shader */
	vshd = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshd, 1, (const GLchar **)&vtx_src, NULL);
	glCompileShader(vshd);

	glGetShaderiv(vshd, GL_COMPILE_STATUS, &success);
	if(!success)
		goto err_cleanup;

	glAttachShader(shaders.prog[slot], vshd);

	/* Load and attach the fragment-shader */
	if(fs_load_file(fs, (uint8_t **)&frg_src, NULL) < 0)
		goto err_cleanup;

	/* Create and initialize the fragment-shader */
	fshd = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshd, 1, (const GLchar **)&frg_src, NULL);
	glCompileShader(fshd);

	glGetShaderiv(fshd, GL_COMPILE_STATUS, &success);
	if(!success)
		goto err_cleanup;

	glAttachShader(shaders.prog[slot], fshd);

	/* Bind the vertex-attributes */
	glBindAttribLocation(shaders.prog[slot], 0, "vtxPos");
	glBindAttribLocation(shaders.prog[slot], 1, "vtxNrm");
	glBindAttribLocation(shaders.prog[slot], 2, "vtxCol");

	/* Link the shader-program */
	glLinkProgram(shaders.prog[slot]);

	/* Detach and destroy shaders */
	glDetachShader(shaders.prog[slot], vshd);
	glDeleteShader(vshd);
	vshd = 0;

	glDetachShader(shaders.prog[slot], fshd);
	glDeleteShader(fshd);
	fshd = 0;

	shaders.mask[slot] = 1;
	strcpy(shaders.name[slot], name);
	free(vtx_src);
	free(frg_src);
	return slot;

err_cleanup:
	if(vshd) {
		glDetachShader(shaders.prog[slot], vshd);
		glDeleteShader(vshd);
	}
	if(fshd) {
		glDetachShader(shaders.prog[slot], fshd);
		glDeleteShader(fshd);
	}

	if(shaders.prog[slot])
		glDeleteProgram(shaders.prog[slot]);

	free(vtx_src);
	free(frg_src);
	return -1;
}

extern void shd_del(short slot)
{
	if(shd_check_slot(slot))
		return;

	if(shaders.mask[slot] == 0)
		return;
	
	glDeleteProgram(shaders.prog[slot]);
	shaders.mask[slot] = 0;
}

extern short shd_get(char *name)
{
	int i;

	for(i = 0; i < SHD_SLOTS; i++) {
		if(shaders.mask[i] == 0)
			continue;

		if(!strcmp(shaders.name[i], name))
			return i;
	}

	return -1;
}

extern void shd_use(short slot, int *loc)
{
	glUseProgram(shaders.prog[slot]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	loc[0] = glGetUniformLocation(shaders.prog[slot], "model");
	loc[1] = glGetUniformLocation(shaders.prog[slot], "view");
	loc[2] = glGetUniformLocation(shaders.prog[slot], "proj");
}

extern void shd_unuse(void)
{
	glUseProgram(0);
}
