#ifndef _VK_H
#define _VK_H

#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>

enum vk_in_attr {
	IN_ATTR_POS = 1 << 0,
	IN_ATTR_TEX = 1 << 1,
	IN_ATTR_NRM = 1 << 2,
	IN_ATTR_JNT = 1 << 3,
	IN_ATTR_WGT = 1 << 4
};

struct vk_pipeline {
	VkDescriptorSetLayout set_layout;
	VkPipelineLayout layout;
	VkPipeline pipeline;
};

struct vk_buffer {
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDeviceSize size;
	void *data;
};

struct vk_texture {
	VkImage image;
	VkDeviceMemory memory;
	VkImageView image_view;
	VkSampler sampler;
};


/*
 * Try to initialize vulkan.
 * 
 * @window: A SDL_Window initialized with SDL_WINDOW_VULKAN
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_init(SDL_Window* window);


/*
 * Destroy vulkan handles.
 */
extern void vk_destroy(void);


/*
 * Resize the vulkan render area to the current size of the window.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_resize(void);


/*
 * Create a new graphics pipeline (equivalent to an OpenGL shader program).
 * 
 * @vtx: The path to the SPIR-V vertex shader
 * @frg: The path to the SPIR-V fragment shader
 * @attr: Flags, which determine the input attributes to the vertex shader
 * @pipeline: A pointer to the pipeline, which will be filled by the function
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_create_pipeline(char *vtx, char *frg, enum vk_in_attr attr,
                              struct vk_pipeline *pipeline);


/*
 * Destroy a graphics pipeline.
 * 
 * @pipeline: The pipeline which will be destroyed
 */
extern void vk_destroy_pipeline(struct vk_pipeline pipeline);


/*
 * Create a descriptor set, which holds all the stuff thats going into the
 * shader that aren't vertex attributes (uniform buffers, textures, etc.)
 * One should be created for each model.
 * 
 * @pipeline: The pipeline of the model
 * @set: A pointer to the VkDescriptorSet, which will be filles by the function
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_create_constant_data(struct vk_pipeline pipeline,
                                   VkDescriptorSet *set);


/*
 * Destroy a descriptor set.
 * 
 * @set: The descriptor set which will be destroyed
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_destroy_constant_data(VkDescriptorSet set);


/*
 * Create a new buffer.
 * 
 * @size: The size of the buffer
 * @usage: The type of the buffer
 * @staging: A boolean which enables memory mapping
 * @buffer: A pointer to the buffer, which will be filled by the function
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                            uint8_t staging, struct vk_buffer *buffer);


/*
 * Copy data to a buffer created with staging enabled.
 * The amount of data is the size of the buffer.
 * 
 * @data: The data, which will be copied.
 * @buffer: The buffer to copy to
 */
extern void vk_copy_data_to_buffer(void* data, struct vk_buffer buffer);


/*
 * Destroy a buffer and free its memory.
 * 
 * @buffer: The buffer which will be destroyed
 */
extern void vk_destroy_buffer(struct vk_buffer buffer);


/*
 * Create a new texture.
 * 
 * @pth: The path to the png of the texture
 * @texture: A pointer to the texture, which will be filled by the function
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_create_texture(char *pth, struct vk_texture *texture);


/*
 * Destroy a texture.
 * 
 * @texture: The texture which will be destroyed
 */
extern void vk_destroy_texture(struct vk_texture texture);


/*
 * Tell the pipeline to use this uniform buffer.
 * Doesn't have to be between start_render() and end_render().
 * 
 * @buffer: The uniform buffer with usage VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
 * @set: The constant data for the model
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_set_uniform_buffer(struct vk_buffer buffer, VkDescriptorSet set);


/*
 * Tell the pipeline to use this texture.
 * Doesn't have to be between start_render() and end_render().
 * 
 * @texture: The texture
 * @set: The constant data for the model
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_set_texture(struct vk_texture texture, VkDescriptorSet set);


/*
 * Start rendering.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_render_start(void);


/*
 * Set the pipeline during rendering.
 * Has to be in between start_render() and end_render().
 * 
 * @pipeline: The pipeline
 */
extern void vk_render_set_pipeline(struct vk_pipeline pipeline);


/*
 * Set the descriptor set during rendering.
 * Has to be in between start_render() and end_render().
 * 
 * @pipeline: The pipeline the model uses
 * @set: The descriptor set
 */
extern void vk_render_set_constant_data(struct vk_pipeline pipeline,
                                        VkDescriptorSet set);


/*
 * Set the vertex buffer during rendering.
 * Has to be in between start_render() and end_render().
 * This buffer must match with the input attributes of the pipeline, e.g
 * first 12 bytes for pos values, second 8 bytes for tex values, etc., but only
 * if you have set IN_ATTR_POS | IN_ATTR_TEX. If, for example, there are no tex
 * values, but nrm values, then the second 12 bytes are for nrm values.
 * 
 * @buffer: The vertex buffer with usage VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
 */
extern void vk_render_set_vertex_buffer(struct vk_buffer buffer);


/*
 * Set the index buffer during rendering.
 * Has to be in between start_render() and end_render().
 * 
 * @buffer: The index buffer with usage VK_BUFFER_USAGE_INDEX_BUFFER_BIT
 */
extern void vk_render_set_index_buffer(struct vk_buffer buffer);


/*
 * Draw the current model during rendering.
 * Has to be in between start_render() and end_render().
 * 
 * @index_count: The amount of indices (not triangles)
 */
extern void vk_render_draw(uint32_t index_count);


/*
 * End rendering and display the result on the screen.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_render_end(void);

/*
 * Print information of the instance, gpu, driver, etc.
 * 
 * Returns: 0 on success or -1 if an error occured
 */
extern int vk_print_info(void);

#endif /* _VK_H */