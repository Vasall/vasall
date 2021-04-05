#ifndef _OPENGL_H
#define _OPENGL_H

#include "matrix.h"
#include "model.h"
#include "rig.h"
#include "sdl.h"

/*
 * Try to initialize opengl.
 * 
 * @window: A SDL_Window initialized with SDL_WINDOW_OPENGL
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int gl_init(SDL_Window *window);


/*
 * Destroy the opengl context.
 */
extern void gl_destroy(void);


/*
 * Create an opengl shader program.
 * 
 * @vs: The path to the vertex shader
 * @fs: The path to the fragment shader
 * @prog: A pointer to the handle of the program, which will be set by the
 *        function
 * @num: The number of elements in the vars array
 * @vars: An array with the names of the input attributes
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int gl_create_program(char *vs, char *fs, uint32_t *prog, int num,
                             char **vars);


/*
 * Delete a shader program.
 * 
 * @prog: The handle of the program
 */
extern void gl_delete_program(uint32_t prog);


/*
 * Create an opengl texture.
 * 
 * @pth: The path to the png file
 * @hdl: A pointer to the handle of the texture, which will be set by the
 *       function
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int gl_create_texture(char *pth, uint32_t *hdl);


/*
 * Destroy a texture.
 * 
 * @hdl: The handle of the texture
 */
extern void gl_destroy_texture(uint32_t hdl);


/*
 * Create a vertex array object.
 * 
 * @vao: A pointer to the handle of the vao, which will be set by the
 *       function
 */
extern void gl_create_vao(uint32_t *vao);


/*
 * Destroy a vertex array object.
 * 
 * @vao: The handle of the vao
 */
extern void gl_destroy_vao(uint32_t vao);


/*
 * Create an opengl buffer.
 * 
 * @vao: The handle of the vao the buffer should be assigned to
 * @type: The type of the buffer
 *        (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, etc.)
 * @size: The size of the buffer
 * @buf: The content of the buffer
 * @bo: A pointer to the handle of the buffer, which will be set by the
 *      function
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int gl_create_buffer(uint32_t vao, int type, size_t size, char *buf,
                            uint32_t *bo);


/*
 * Set the content of a buffer.
 *
 * @type: The type of the buffer (GL_UNIFORM_BUFFER, GL_ARRAY_BUFFER, etc.)
 * @buffer: The handle of the buffer
 * @size: The size of the content
 * @data: The new content of the buffer
 */
extern void gl_set_buffer_data(int type, unsigned int buffer, int size,
			       void* data);


/*
 * Destroy a buffer.
 * 
 * @bo: The handle of the buffer object
 */
extern void gl_destroy_buffer(uint32_t bo);


/*
 * Tell opengl, where each vertex input attribute is in the vertex buffer.
 * 
 * @vao: The handle of the vao of the vertex buffer
 * @vbo: The handle of the vertex buffer
 * @stride: The size of one full vertex in the array
 * @rig: A boolean to tell if the model is animated
 */
extern void gl_set_input_attr(uint32_t vao, uint32_t vbo, int stride, int rig);


/*
 * Resize the opengl viewport.
 * 
 * @w: The new with of the window
 * @h: The new height of the window
 */
extern void gl_resize(int w, int h);


/*
 * Start rendering.
 */
extern void gl_render_start(void);


/*
 * Set the program during rendering.
 * 
 * @prog: The handle of the shader program
 * @attr: The amount of input attributes
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int gl_render_set_program(uint32_t prog, int attr);


/*
 * Set the vao during rendering.
 * 
 * @vao: The handle of the vao
 */
extern void gl_render_set_vao(uint32_t vao);


/*
 * Set the texture during rendering.
 * 
 * @hdl: the handle of the texture
 */
extern void gl_render_set_texture(uint32_t hdl);


/*
 * Set the values of the uniform variables during rendering.
 * 
 * @buf: The handle of the uniform buffer
 * @light: The light buffer of the world
 * @uni: The uniform buffer data
 */
extern void gl_render_set_uniform_buffer(unsigned int buf, unsigned int light,
                                         struct uni_buffer uni);


/*
 * Draw the model.
 * 
 * @indices: The amount of indices
 */
extern void gl_render_draw(size_t indices);


/*
 * End rendering and display the result ot the screen.
 * 
 * @window: The window to display on
 */
extern void gl_render_end(SDL_Window *window);

/*
 * Print information on version, gpu, vendor, etc.
 */
extern void gl_print_info(void);

#endif /* _OPENGL_H */
