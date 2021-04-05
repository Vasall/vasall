#include "opengl.h"

#include "error.h"
#include "filesystem.h"
#include "model.h"

struct opengl_wrapper {
	SDL_GLContext context;
};

static struct opengl_wrapper ogl;


extern int gl_init(SDL_Window *window)
{
	ogl.context = SDL_GL_CreateContext(window);
	if(ogl.context == NULL) {
		ERR_LOG(("Couldn't create opengl context"));
		return -1;
	}

	/* glClearColor(0.196, 0.235, 0.282, 1.0); */
	glClearColor(0.094, 0.094, 0.094, 1.0);
	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	return 0;
}


extern void gl_destroy(void)
{
	SDL_GL_DeleteContext(ogl.context);
}


extern int gl_create_program(char *vs, char *fs, uint32_t *prog, int num,
                             char **vars)
{
	int i;
	int success;
	char infoLog[512];
	uint32_t fshd = 0;
	uint32_t vshd = 0;
	char *vtx_src = NULL;
	char *frg_src = NULL;
	GLint isLinked = 0;
	unsigned int loc;

	if((*prog = glCreateProgram()) == 0) {
		ERR_LOG(("Failed to create shader-program"));
		goto err_cleanup;
	}

	/* Load vertex-shader and attach the vertex-shader */
	if(fs_load_file(vs, (uint8_t **)&vtx_src, NULL) < 0) {
		ERR_LOG(("Failed to load vtx-shader: %s", vs));
		goto err_cleanup;
	}

	/* Create and initialize the vertex-shader */
	vshd = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshd, 1, (const GLchar **)&vtx_src, NULL);
	glCompileShader(vshd);

	/* Check if vertex-shader compiled successfully */
	glGetShaderiv(vshd, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(vshd, 512, NULL, infoLog);
		ERR_LOG(("Failed to compile shader"));
		printf("  %s: %s", vs, infoLog);
		goto err_cleanup;
	}

	glAttachShader(*prog, vshd);

	/* Load and attach the fragment-shader */
	if(fs_load_file(fs, (uint8_t **)&frg_src, NULL) < 0) {
		ERR_LOG(("Failed to load frg-shader: %s", fs));
		goto err_cleanup;
	}

	/* Create and initialize the fragment-shader */
	fshd = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshd, 1, (const GLchar **)&frg_src, NULL);
	glCompileShader(fshd);

	/* Check if fragment-shader compiled successfully */
	glGetShaderiv(fshd, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(fshd, 512, NULL, infoLog);
		ERR_LOG(("Failed to compile shader"));
		printf("  %s: %s", fs, infoLog);
		goto err_cleanup;
	}

	glAttachShader(*prog, fshd);

	/* Bind the vertex-attributes */
	for(i = 0; i < num; i++)
		glBindAttribLocation(*prog, i, vars[i]);

	if(glGetError() != GL_NO_ERROR)
		ERR_LOG(("Failed to bind attribute-location"));

	/* Link the shader-program */
	glLinkProgram(*prog);

	/* Check if shader-program has been linked successfully */
	glGetProgramiv(*prog, GL_LINK_STATUS, (int *)&isLinked);
	if(isLinked == GL_FALSE) {
		GLchar infoLog[512];
        	GLint size;
        	glGetProgramInfoLog(*prog, 512, &size, infoLog);
		ERR_LOG(("Failed to link shader: %s", infoLog));
	}

	loc = glGetUniformBlockIndex(*prog, "UBO");
	if(loc != GL_INVALID_INDEX)
		glUniformBlockBinding(*prog, loc, 0);

	loc = glGetUniformBlockIndex(*prog, "light");
	if(loc != GL_INVALID_INDEX)
		glUniformBlockBinding(*prog, loc, 2);

	/* Detach and destroy assets.shd */
	glDetachShader(*prog, vshd);
	glDeleteShader(vshd);
	vshd = 0;

	glDetachShader(*prog, fshd);
	glDeleteShader(fshd);
	fshd = 0;

	free(vtx_src);
	free(frg_src);
	return 0;

err_cleanup:
	if(vshd) {
		glDetachShader(*prog, vshd);
		glDeleteShader(vshd);
	}
	if(fshd) {
		glDetachShader(*prog, fshd);
		glDeleteShader(fshd);
	}

	if(*prog)
		glDeleteProgram(*prog);

	free(vtx_src);
	free(frg_src);
	return -1;
}


extern void gl_delete_program(uint32_t prog)
{
	glDeleteProgram(prog);
}


extern int gl_create_texture(char *pth, uint32_t *hdl)
{
	int w, h;
	uint8_t *px;
	float ani;

	if(fs_load_png(pth, &px, &w, &h) < 0) {
		ERR_LOG(("Failed to load texture: %s", pth));
		return -1;
	}

	glGenTextures(1, hdl);
	glBindTexture(GL_TEXTURE_2D, *hdl);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, px);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &ani);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, ani);

	glBindTexture(GL_TEXTURE_2D, 0);

	free(px);
	return 0;
}


extern void gl_destroy_texture(uint32_t hdl)
{
	glDeleteTextures(1, &hdl);
}

extern void gl_create_vao(uint32_t *vao)
{
	glGenVertexArrays(1, vao);
}


extern void gl_destroy_vao(uint32_t vao)
{
	glDeleteVertexArrays(1, &vao);
}


extern int gl_create_buffer(uint32_t vao, int type, size_t size, char *buf,
                            uint32_t *bo)
{
	int err;

	/* Bind vertex-array-object */
	glBindVertexArray(vao);

	glGenBuffers(1, bo);
	glBindBuffer(type, *bo);
	glBufferData(type, size, buf, GL_STATIC_DRAW);

	/* Unbind the vertex-array-object */
	glBindVertexArray(0);

	if((err = glGetError()) != GL_NO_ERROR) {
		ERR_LOG(("Failed to create buffer"));
		return -1;
	}
	return 0;
}


extern void gl_set_buffer_data(int type, unsigned int buffer, int size,
			       void* data)
{
	glBindBuffer(type, buffer);
	glBufferData(type, size, data, GL_STATIC_DRAW);
	glBindBuffer(type, 0);
}


extern void gl_destroy_buffer(uint32_t bo)
{
	glDeleteBuffers(1, &bo);
}


extern void gl_set_input_attr(uint32_t vao, uint32_t vbo, int stride, int rig)
{
	void *p;

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	/* Bind the data to the vertices */
	/* Vertex-Position */
	p = NULL;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, p);

	/* Tex-Coordinate */
	p = (void *)(3 * sizeof(float));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, p);

	/* Normal-Vector */
	p = (void *)(5 * sizeof(float));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, p);

	if(rig) {
		/* Joint-Index */
		p = (void *)(8 * sizeof(float));
		glVertexAttribIPointer(3, 4, GL_INT, stride, p);

		/* Joint-Weights */
		p = (void *)(8 * sizeof(float) + 4 * sizeof(int));
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, p);
	}
}


extern void gl_resize(int w, int h)
{
	glViewport(0, 0, w, h);
}


extern void gl_render_start(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


extern int gl_render_set_program(uint32_t prog, int attr)
{
	int i;

	/* Use the attached shader program */
	glUseProgram(prog);

	/* Enable the vertices up to attr */
	for(i = 0; i < attr; i++)
		glEnableVertexAttribArray(i);

	if(glGetError() != GL_NO_ERROR) {
		ERR_LOG(("Failed to enable attribute"));
		return -1;
	}

	if(glGetError() != GL_NO_ERROR) {
		ERR_LOG(("Failed to get locations"));
		return -1;
	}

	return 0;
}


extern void gl_render_set_vao(uint32_t vao)
{
	glBindVertexArray(vao);
}


extern void gl_render_set_texture(uint32_t hdl)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, hdl);
}


extern void gl_render_set_uniform_buffer(unsigned int buf, unsigned int light,
                                         struct uni_buffer uni)
{

	glBindBuffer(GL_UNIFORM_BUFFER, buf);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(struct uni_buffer), &uni,
				GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, buf);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, light);
}


extern void gl_render_draw(size_t indices)
{
	glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, NULL);
}


extern void gl_render_end(SDL_Window *window)
{
	SDL_GL_SwapWindow(window);
}


extern void gl_print_info(void)
{
	const unsigned char *ver;
	const unsigned char *vendor;
	const unsigned char *gpu;
	const unsigned char *glsl_ver;
	
	ver = glGetString(GL_VERSION);
	vendor = glGetString(GL_VENDOR);
	gpu = glGetString(GL_RENDERER);
	glsl_ver = glGetString(GL_SHADING_LANGUAGE_VERSION);

	printf("------------------ OpenGL Info -------------------\n");
	printf("GL Version: %s\n", ver);
	printf("Vendor: %s\n", vendor);
	printf("GPU: %s\n", gpu);
	printf("GLSL Version: %s\n", glsl_ver);
	printf("--------------------------------------------------\n");
}
