# New renderer design - Draft

## Main functions
```c
int ren_init(SDL_Window **window, int width, int height, const char *title);
```
```c
int ren_prepare();
```
```c
int ren_draw();
```
```c
int ren_resize();
```
```c
int ren_destroy();
```
- `ren_init()` should be called once at the beginnning to initialize the window and the renderer
- `ren_prepare()` should be called in the main loop after updating the gpu data. This function sorts the data and uploads it to gpu memory
- `ren_draw()` draws a frame and displays it. Should be called as often as possible
- `ren_resize()` should be called for window resizing and applying video settings
- `ren_destroy()` removes the stuff initialized with `ren_init()`

## Datatypes
```c
union ren_pipeline {
	struct {
		VkDescriptorSetLayout set_layout;
		VkPipelineLayout layout;
		VkPipeline pipeline;
	} vk;
	GLuint gl;
};
```
```c
struct material {
	union ren_pipeline pipeline;
	float roughness;
	...
};
```
```c
union ren_buf {
	struct {
		VkBuffer buf;
		struct mem_block mem;
	} vk;
	GLuint gl;
};
```
```c
struct ren_mesh {
	struct ren_buf vertices;
	struct ren_buf indices;
	GLuint vao;
};
```
```c
union ren_texture {
	struct {
		VkImage image;
		VkImageView view;
		struct mem_block mem;
	} vk;
	GLuint gl;
};
```
```c
struct object {
	short material;
	short mesh;
	short texture;
	VkDescriptorSet set;
	mat4_t mdl_mat;
};
```

## object functions
### pipeline
```c
int ren_create_pipeline(union ren_pipeline *pipeline);
```
```c
int ren_destroy_pipeline(union ren_pipeline pipeline);
```
### mesh
```c
int ren_create_mesh(struct ren_mesh *mesh);
```
```c
int ren_update_mesh(struct ren_mesh *mesh);
```
```c
int ren_destroy_mesh(struct ren_mesh mesh);
```
### texture
```c
int ren_create_texture(union ren_texture *texture);
```
```c
int ren_update_texture(union ren_texture *texture);
```
```c
int ren_destroy_texture(union ren_texture texture);
```
### global shader data
```c
int ren_set_global(void *data, size_t size);
```

## GPU memory managment (Vulkan only?)
Big buffers for vertices, indices, textures, model matrices  
The `ren_create_*` and `ren_destroy_*` functions actually don't upload to GPU memory directly, but rather queue for upload, which happens in `ren_prepare()`. This means, the data can't be freed immediatly after calling the `ren_create_*` function, but after `ren_prepare()`. Memory Managment will be a bit like malloc(). Specifically, it will allocate a larger block of memoy than needed and store the offset and the size for one mesh in a data structure. When creating a new mesh, the data will be put anywhere, where there is available memory, if there's space for it. When destroying a mesh, the data doesn't vanish from the gpu memory. It will just be marked as available. More details below. To minimize the data transfer, only the data that changed will be uploaded to GPU.
### Memory Allocation
```c
struct mem_block {
	uint32_t mem_type;
	VkDeviceMemory memory;
	uint32_t offset;
	size_t size;
};
```
```c
enum mem_type {
	MEM_TYPE_CPU,
	MEM_TYPE_CPU_GPU
};
```
```c
int vmalloc(VkMemoryRequirements req, enum mem_type, struct mem_block *mem);
```
### Free Memory
```c
int vmfree(struct mem_block mem);
```
### Internals
```c
struct mem_free {
	struct mem_block mem;
	struct mem_free *next;	
};
```
```c
struct mem_free *head;
struct mem_free *tail;
```
In `vmalloc()`, the program will go through the mem_free linked list and search for available memory. It will compare the memory type and size, so that the new object can fit in it. If it finds a suitable memory block, the program will return the memory block with the same offset and adjusted size. If the size of the available memory block was bigger than the required size of the new object, then the program will adjust the offset and the size in the mem_free item. Otherwise the mem_free item will just be deleted. If no suitable memory block could be found, the program then allocates a big block of video memory and appends the remainig free memory after the assignment to the new object to the mem_free linked list.

`vmfree()` takes the memory block and appends it to the linked list.

**There has to be a function that runs periodically and checks the linked list if a VkDeviceMemory object is completely available and then frees it.**

## Vulkan Specifics
### Image
```c
enum image_flags {
	IMAGE_MIP_LEVELS = 0x1,
	IMAGE_CUBE_MAP = 0x2,
	IMAGE_CHANGEABLE = 0x4,
	IMAGE_SAMPLES = 0x8
};
```
- `IMAGE_MIP_LEVELS` enables mipmap generation of a texture
- `IMAGE_CUBE_MAP` specifies the image as a cubemap
- `IMAGE_CHANGEBALE` for ui when the texture isn't static
- `IMAGE_SAMPLES` for framebuffer images, when there is multisampling
```c
static int create_image(VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usage, enum image_flags flags, VkImage *image, struct mem_block *mem);
```