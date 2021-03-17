#ifndef _RENDER_ENGINE_H
#define _RENDER_ENGINE_H

#include "vulkan.h"
#include "opengl.h"


enum render_mode {
	REN_MODE_OPENGL,
	REN_MODE_VULKAN
};


/*
 * The render wrapper.
 */
struct render_wrapper {
	enum render_mode mode;
};

/* Define the global render-wrapper */
extern struct render_wrapper renderer;


/*
 * Select and initialize the rendering engine.
 * If the function returns with -1 destroy the window and call this function
 * again with a SDL_Window initialized with SDL_WINDOW_OPENGL.
 *
 * @window: A SDL_Window initialized with SDL_WINDOW_VULKAN 
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int ren_init(SDL_Window *window);


/*
 * Destroy render engine.
 * Make sure every other pipeline, texture, buffer, etc. was previously
 * destroyed.
 */
extern void ren_destroy(void);


/*
 * Resize the rendering space.
 */
extern int ren_resize(int w, int h);


/*
 * Create a shader program/pipeline.
 * 
 * @vs: The path to the vertex shader
 * @fs: The path to the fragment shader
 * @prog: A pointer to the handle of the opengl program, which will be set by
 *        the function
 * @pipeline: A pointer to the vulkan pipeline, which will be filled by the
 *            function
 * @num: The number of elements in the vars array
 * @vars: An array with the names of the input attributes
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int ren_create_shader(char *vs, char *fs, uint32_t *prog,
                             struct vk_pipeline *pipeline, int num,
                             char **vars);


/*
 * Destroy a shader program/pipeline.
 * 
 * @prog: The handle of the opengl program
 * @pipeline: The pipeline
 */
extern void ren_destroy_shader(uint32_t prog, struct vk_pipeline pipeline);


/*
 * Create a new texture.
 * 
 * @pth: The path to the PNG of the texture
 * @hdl: A pointer to the handle of the opengl texture, which will be set by
 *       the function
 * @texture: A pointer to the vulkan texture, which will be filled by the
 *           function
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int ren_create_texture(char *pth, uint32_t *hdl,
                              struct vk_texture *texture);


/*
 * Destroy a texture.
 * 
 * @hdl: The opengl handle of the texture
 * @texture: The vulkan texture
 */
extern void ren_destroy_texture(uint32_t hdl, struct vk_texture texture);


/*
 * Create an opengl vertex array object or a vulkan decsriptor set and a vulkan
 * uniform buffer.
 * 
 * @pipeline: The vulkan pipeline
 * @vao: A pointer to the handle of the opengl vao, which will be set by
 *         the function
 * @set: A pointer to the vulkan descriptor set, which will be filled by the
 *       function
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int ren_create_model_data(struct vk_pipeline pipeline, uint32_t *vao,
                                 VkDescriptorSet *set);


/*
 * Destroy the vao or the descriptor set.
 * 
 * @vao: The opengl handle of the vao
 * @set: The vulkan descriptor set
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int ren_destroy_model_data(uint32_t vao, VkDescriptorSet set);


/*
 * Create a buffer.
 * 
 * @vao: The handle of the opengl vao the buffer should be assigned to
 * @type: The type of the buffer can be:
 *        GL_ARRAY_BUFFER for vertex buffer
 *        GL_ELEMENT_ARRAY_BUFFER for index buffer
 * @size: The size of the buffer
 * @buf: The content of the buffer with length 'size'
 * @bo: A pointer to the handle of the opengl buffer, which will be set by
 *      the function
 * @buffer: A pointer to the vulkan buffer, which will be filled by the function
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int ren_create_buffer(uint32_t vao, int type, size_t size, char *buf,
                             uint32_t *bo, struct vk_buffer *buffer);


/*
 * Destroy a buffer.
 * 
 * @bo: The opengl handle of the buffer
 * @buffer: The vulkan buffer
 */
extern void ren_destroy_buffer(uint32_t bo, struct vk_buffer buffer);


/*
 * Set the opengl vertex input attributes or the vulkan uniform buffer and
 * texture of a model.
 * 
 * @vao: The opengl handle of the vao
 * @vbo: The opengl handle of the vertex buffer
 * @stride: The size of one full vertex in the array
 * @rig: A boolean to tell if the model is animated
 * @set: The vulkan descriptor set
 * @uniform_buffer: The vulkan uniform buffer
 * @texture: The vulkan texture
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int ren_set_model_data(uint32_t vao, uint32_t vbo, int stride, int rig,
                              VkDescriptorSet set,
                              struct vk_buffer uniform_buffer,
                              struct vk_texture texture);


/*
 * Start rendering.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int ren_start(void);


/*
 * Set the shader program/pipeline during rendering.
 * 
 * @prog: The opengl handle of the program
 * @attr: The amount of input attributes
 * @pipeline: The vulkan pipeline
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int ren_set_shader(uint32_t prog, int attr, struct vk_pipeline pipeline);


/*
 * Set the vertex and index buffers during rendering.
 * 
 * @vao: The opengl handle of the vao
 * @vtx_buffer: The vulkan vertex buffer
 * @idx_buffer: The vulkan index buffer
 */
extern void ren_set_vertices(uint32_t vao, struct vk_buffer vtx_buffer,
                             struct vk_buffer idx_buffer);


/*
 * Set the opengl uniform buffer and texture or the vulkan descriptor set.
 * 
 * @uni_buf: The opengl handle of the uniform buffer
 * @uni: The uniform buffer data
 * @hdl: The opengl handle of the texture
 * @pipeline: The vulkan pipeline
 * @vk_uni_buf: The vulkan uniform buffer
 * @set: The vulkan descriptor set
 */
extern void ren_set_render_model_data(unsigned int uni_buf,
                                      struct uni_buffer uni, uint32_t hdl,
                                      struct vk_pipeline pipeline,
                                      struct vk_buffer vk_uni_buf,
                                      VkDescriptorSet set);


/*
 * Draw the model.
 * 
 * @indices: The amount of indices
 */
extern void ren_draw(uint32_t indices);


/*
 * End rendering and display the result ot the screen.
 * 
 * @window: The window to display on
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int ren_end(SDL_Window *window);

/*
 * Print various information on api version, gpu, etc.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int ren_print_info(void);

#endif /* _RENDER_ENGINE_H */
