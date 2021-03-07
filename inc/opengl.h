#ifndef OPENGL_H
#define OPENGL_H

#include "matrix.h"
#include "model.h"
#include "rig.h"
#include "sdl.h"

/**
 * Try to initialize opengl
 * 
 * @window: a SDL_Window initialized with SDL_WINDOW_OPENGL
 */
extern int gl_init(SDL_Window *window);

/**
 * Destroy the opengl context
 */
extern int gl_destroy(void);

/**
 * Create an opengl shader program
 * 
 * @vs: the path to the vertex shader
 * @fs: the path to the fragment shader
 * @prog: a pointer to the handle of the program, which will be set by the
 * 		function
 * @num: the number of elements in the vars array
 * @vars: an array with the names of the input attributes
 */
extern int gl_create_program(char *vs, char *fs, uint32_t *prog, int num,
					char **vars);

/**
 * Delete a shader program
 * 
 * @prog: the handle of the program
 */
extern int gl_delete_program(uint32_t prog);

/**
 * Create an opengl texture
 * 
 * @pth: the path to the png file
 * @hdl: a pointer to the handle of the texture, which will be set by the
 * 		function
 */
extern int gl_create_texture(char *pth, uint32_t *hdl);

/**
 * Destroy a texture
 * 
 * @hdl: the handle of the texture
 */
extern int gl_destroy_texture(uint32_t hdl);

/**
 * Create a vertex array object
 * 
 * @vao: a pointer to the handle of the vao, which will be set by the
 * 		function
 */
extern int gl_create_vao(uint32_t *vao);

/**
 * Destroy a vertex array object
 * 
 * @vao: the handle of the vao
 */
extern int gl_destroy_vao(uint32_t vao);

/**
 * Create an opengl buffer
 * 
 * @vao: the handle of the vao the buffer should be assigned to
 * @type: the type of the buffer
 * 		(GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, etc.)
 * @size: the size of the buffer
 * @buf: the content of the buffer
 * @bo: a pointer to the handle of the buffer, which will be set by the
 * 		function
 */
extern int gl_create_buffer(uint32_t vao, int type, size_t size, char *buf,
					uint32_t *bo);

/**
 * Destroy a buffer
 * 
 * @bo: the handle of the buffer object
 */
extern int gl_destroy_buffer(uint32_t bo);

/**
 * Tell opengl, where each vertex input attribute is in the vertex buffer
 * 
 * @vao: the handle of the vao of the vertex buffer
 * @vbo: the handle of the vertex buffer
 * @stride: the size of one full vertex in the array
 * @rig: a boolean to tell if the model is animated
 */
extern int gl_set_input_attr(uint32_t vao, uint32_t vbo, int stride, int rig);

/**
 * Resize the opengl viewport
 * 
 * @w: the new with of the window
 * @h: the new height of the window
 */
extern int gl_resize(int w, int h);

/**
 * Start rendering
 */
extern int gl_render_start(void);

/**
 * Set the program during rendering
 * 
 * @prog: the handle of the shader program
 * @attr: the amount of input attributes
 */
extern int gl_render_set_program(uint32_t prog, int attr);

/**
 * Set the vao during rendering
 * 
 * @vao: the handle of the vao
 */
extern int gl_render_set_vao(uint32_t vao);

/**
 * Set the texture during rendering
 * 
 * @hdl: the handle of the texture
 */
extern int gl_render_set_texture(uint32_t hdl);

/**
 * Set the values of the uniform variables during rendering
 * 
 * @buf: the handle of the uniform buffer
 * @uni: the uniform buffer data
 */
extern int gl_render_set_uniform_buffer(unsigned int buf,
				struct uni_buffer uni);

/**
 * Draw the model
 * 
 * @indices: the amount of indices
 */
extern int gl_render_draw(size_t indices);

/**
 * End rendering and display the result ot the screen
 * 
 * @window: the window to display on
 */
extern int gl_render_end(SDL_Window *window);

#endif /* OPENGL_H */