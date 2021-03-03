#ifndef VK_H
#define VK_H

#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>

enum input_attributes {
	IN_ATTR_POS = 1 << 0,
	IN_ATTR_TEX = 1 << 1,
	IN_ATTR_NRM = 1 << 2,
	IN_ATTR_JGT = 1 << 3,
	IN_ATTR_WGT = 1 << 4
};

struct pipeline_wrapper {
	VkDescriptorSetLayout set_layout;
	VkPipelineLayout layout;
	VkPipeline pipeline;
};

struct buffer_wrapper {
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDeviceSize size;
	void *data;
};

struct texture_wrapper {
	VkImage image;
	VkDeviceMemory memory;
	VkImageView image_view;
	VkSampler sampler;
};

/*
 * Try to initialize vulkan
 * 
 * @window: a SDL_Window initialized with SDL_WINDOW_VULKAN
 */
int init_vulkan(SDL_Window* window);

/*
 * Destroy vulkan handles
 */
int destroy_vulkan(void);

/*
 * Create a new graphics pipeline (equivalent to an OpenGL shader program)
 * 
 * @vtx: the path to the SPIR-V vertex shader
 * @frg: the path to the SPIR-V fragment shader
 * @attr: flags, which determines the input attributes to the vertex shader
 * @wrapper: A pointer to the pipeline, which will be filled by the function
 */
int create_pipeline(char *vtx, char *frg, enum input_attributes attr,
							struct pipeline_wrapper *wrapper);

/*
 * Destroy a graphics pipeline
 * 
 * @wrapper: the pipeline which will be destroyed
 */
int destroy_pipeline(struct pipeline_wrapper wrapper);

/*
 * Create a descriptor set, which holds all the stuff thats going into the
 * shader that aren't vertex attributes (uniform buffers, textures, etc.)
 * One should be created for each model
 * 
 * @pipeline: the pipeline of the model
 * @set: a pointer to the Descriptor Set, which will be filles by the function
 */
int create_constant_data(struct pipeline_wrapper pipeline,
							VkDescriptorSet *set);

/*
 * Destroy a descriptor set
 * 
 * @set: the descriptor set which will be destroyed
 */
int destroy_constant_data(VkDescriptorSet set);

/*
 * Create a new buffer
 * 
 * @size: the size of the buffer
 * @usage: the type of the buffer
 * @staging: a boolean which enables memory mapping
 * @wrapper: A pointer to the buffer, which will be filled by the function
 */
int create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, uint8_t staging,
					struct buffer_wrapper *wrapper);

/*
 * Copy data to a buffer created with staging enabled and unmap the memory
 * The amount of data is the size of the buffer
 * 
 * @data: the data, which will be copied.
 * @buffer: the buffer to copy to
 */
int copy_data_to_buffer(void* data, struct buffer_wrapper buffer);

/*
 * Destroy a buffer and free its memory
 * 
 * @wrapper: the buffer which will be destroyed
 */
int destroy_buffer(struct buffer_wrapper wrapper);

/*
 * Create a new texture
 * 
 * @pth: the path to the png of the texture
 * @wrapper: A pointer to the texture, which will be filled by the function
 */
int create_texture(char *pth, struct texture_wrapper *wrapper);

/*
 * Destroy a texture
 * 
 * @wrapper: the texture which will be destroyed
 */
int destroy_texture(struct texture_wrapper wrapper);

/*
 * Tell the pipeline to use this uniform buffer
 * Doesn't have to be between start_render() and end_render()
 * 
 * @buffer: the uniform buffer with usage VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
 * @set: the constant data for the model
 */
int set_uniform_buffer(struct buffer_wrapper buffer, VkDescriptorSet set);

/*
 * Tell the pipeline to use this texture
 * Doesn't have to be between start_render() and end_render()
 * 
 * @texture: the texture
 * @set: the the constant data for the model
 */
int set_texture(struct texture_wrapper texture, VkDescriptorSet set);

/*
 * Start rendering
 */
int start_render(void);

/*
 * Set the pipeline during rendering
 * Has to be in between start_render() and end_render()
 * 
 * @pipeline: the pipeline
 */
int render_set_pipeline(struct pipeline_wrapper pipeline);

/*
 * Set the descriptor set during rendering
 * Has to be in between start_render() and end_render()
 * 
 * @pipeline: the pipeline the model uses
 * @set: the descriptor set
 */
int render_set_constant_data(struct pipeline_wrapper pipeline,
					VkDescriptorSet set);

/*
 * Set the vertex buffers during renndering
 * Has to be in between start_render() and end_render()
 * These buffers must match with the input attributes of the pipeline, e.g
 * first buffer for pos values, second buffer for tex values, etc., but only if
 * you have set IN_ATTR_POS | IN_ATTR_TEX. If, for example, there are no tex
 * values, but nrm values, the the second buffer becomes the buffer for nrm
 * values
 * 
 * @buffer_count: the amount of buffers in the buffers array
 * @buffers: an array of buffers with usage VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
 */
int render_set_vertex_buffers(int buffer_count, struct buffer_wrapper *buffers);

/*
 * Set the index buffer during rendering
 * Has to be in between start_render() and end_render()
 * 
 * @buffer: the index buffer with usage VK_BUFFER_USAGE_INDEX_BUFFER_BIT
 */
int render_set_index_buffer(struct buffer_wrapper buffer);

/*
 * Draw the current model during rendering
 * Has to be in between start_render() and end_render()
 * 
 * @index_count: the amount of indices (not triangles)
 */
int render_draw(uint32_t index_count);

/*
 * End rendering and display the result ot the screen
 */
int end_render(void);

#endif /* VK_H */