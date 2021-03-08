#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include "vulkan.h"
#include "opengl.h"

/**
 * Select and initialize the rendering engine
 * 
 * @window: a SDL_Window initialized with SDL_WINDOW_VULKAN
 * 
 * If the function returns with -1 destroy the window and call this function
 * again with a SDL_Window initialized with SDL_WINDOW_OPENGL
 */
extern int ren_init(SDL_Window *window);

/**
 * Destroy render engine
 * Make sure every other pipeline, texture, buffer, etc. was previously
 * destroyed
 */
extern int ren_destroy(void);

/**
 * Resize the rendering space
 */
extern int ren_resize(int w, int h);

/**
 * Create a shader program/pipeline
 * 
 * @vs: the path to the vertex shader
 * @fs: the path to the fragment shader
 * @prog: a pointer to the handle of the opengl program, which will be set by
 * 		the function
 * @pipeline: a pointer to the vulkan pipeline, which will be filled by the
 * 		function
 * @num: the number of elements in the vars array
 * @vars: an array with the names of the input attributes
 */
extern int ren_create_shader(char *vs, char *fs, uint32_t *prog,
		struct vk_pipeline *pipeline, int num, char **vars);

/**
 * Destroy a shader program/pipeline
 * 
 * @prog: the handle of the opengl program
 * @pipeline: the pipeline
 */
extern int ren_destroy_shader(uint32_t prog, struct vk_pipeline pipeline);

/**
 * Create a texture
 * 
 * @pth: the path to png of the texture
 * @hdl: a pointer to the handle of the opengl texture, which will be set by
 * 		the function
 * @texture: a pointer to the vulkan texture, which will be filled by the
 * 		function
 */
extern int ren_create_texture(char *pth, uint32_t *hdl,
			struct vk_texture *texture);

/**
 * Destroy a texture
 * 
 * @hdl: the opengl handle of the texture
 * @texture: the vulkan texture
 */
extern int ren_destroy_texture(uint32_t hdl, struct vk_texture texture);

/**
 * Create an opengl vertex array object or a vulkan decsriptor set and a vulkan
 * uniform buffer
 * 
 * @pipeline: the vulkan pipeline
 * @vao: a pointer to the handle of the opengl vao, which will be set by
 * 		the function
 * @set: a pointer to the vulkan descriptor set, which will be filled by the
 * 		function
 */
extern int ren_create_model_data(struct vk_pipeline pipeline, uint32_t *vao,
				VkDescriptorSet *set);

/**
 * Destroy the vao or the descriptor set
 * 
 * @vao: the opengl handle of the vao
 * @set: the vulkan descriptor set
 */
extern int ren_destroy_model_data(uint32_t vao, VkDescriptorSet set);

/**
 * Create a buffer
 * 
 * @vao: the handle of the opengl vao the buffer should be assigned to
 * @type: the type of the buffer can be:
 * 		GL_ARRAY_BUFFER for vertex buffer
 * 		GL_ELEMENT_ARRAY_BUFFER for index buffer
 * @size: the size of the buffer
 * @buf: the content of the buffer with length 'size'
 * @bo: a pointer to the handle of the opengl buffer, which will be set by
 * 		the function
 * @buffer: a pointer to the vulkan buffer, which will be filled by the function
 */
extern int ren_create_buffer(uint32_t vao, int type, size_t size, char *buf,
				uint32_t *bo, struct vk_buffer *buffer);

/**
 * Destroy a buffer
 * 
 * @bo: the opengl handle of the buffer
 * @buffer: the vulkan buffer
 */
extern int ren_destroy_buffer(uint32_t bo, struct vk_buffer buffer);

/**
 * Set the opengl vertex input attributes or the vulkan uniform buffer and
 * texture of a model
 * 
 * @vao: the opengl handle of the vao
 * @vbo: the opengl handle of the vertex buffer
 * @stride: the size of one full vertex in the array
 * @rig: a boolean to tell if the model is animated
 * @set: the vulkan descriptor set
 * @uniform_buffer: the vulkan uniform buffer
 * @texture: the vulkan texture
 */
extern int ren_set_model_data(uint32_t vao, uint32_t vbo, int stride, int rig,
	VkDescriptorSet set, struct vk_buffer uniform_buffer,
	struct vk_texture texture);

/**
 * Start rendering
 */
extern int ren_start(void);

/**
 * Set the shader program/pipeline during rendering
 * 
 * @prog: the opengl handle of the program
 * @attr: the amount of input attributes
 * @pipeline: the vulkan pipeline
 */
extern int ren_set_shader(uint32_t prog, int attr, struct vk_pipeline pipeline);

/**
 * Set the vertex and index buffers during rendering
 * 
 * @vao: the opengl handle of the vao
 * @vtx_buffer: the vulkan vertex buffer
 * @idx_buffer: the vulkan index buffer
 */
extern int ren_set_vertices(uint32_t vao, struct vk_buffer vtx_buffer,
			struct vk_buffer idx_buffer);

/**
 * Set the opengl uniform buffer and texture or the vulkan descriptor set
 * 
 * @uni_buf: the opengl handle of the uniform buffer
 * @uni: the uniform buffer data
 * @hdl: the opengl handle of the texture
 * @pipeline: the vulkan pipeline
 * @vk_uni_buf: the vulkan uniform buffer
 * @set: the vulkan descriptor set
 */
extern int ren_set_render_model_data(unsigned int uni_buf,
			struct uni_buffer uni, uint32_t hdl, struct vk_pipeline pipeline,
			struct vk_buffer vk_uni_buf, VkDescriptorSet set);

/**
 * Draw the model
 * 
 * @indices: the amount of indices
 */
extern int ren_draw(uint32_t indices);

/**
 * End rendering and display the result ot the screen
 * 
 * @window: the window to display on
 */
extern int ren_end(SDL_Window *window);

#endif /* RENDER_ENGINE_H */