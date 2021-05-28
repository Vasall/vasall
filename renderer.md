# New renderer design - Draft

## Main functions
```c
int ren_init();
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
- `ren_init()` should be called once at the beginnning to initialize the renderer
- `ren_prepare()` should be called in the main loop after updating the gpu data. This function sorts the data and uploads it to gpu memory
- `ren_draw()` draws a frame and displays it. Should be called as often as possible
- `ren_resize()` should be called for window resizing and applying video settings
- `ren_destroy()` removes the stuff initialized with `ren_init()`

## Datatypes
```c
union ren_pipeline {
	struct vk {
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
struct ren_buf {
	uint32_t offset;
	size_t size;
	union buf {
		VkBuffer vk;
		GLuint gl;
	} buf;
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
	struct vk {
		VkImage image;
		VkImageView view;
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

## GPU memory managment
Big buffers for vertices, indices, textures, model matrices  
The `ren_create_*` and `ren_destroy_*` functions actually don't upload to GPU memory directly, but rather queue for upload, which happens in `ren_prepare()`. This means, the data can't be freed immediatly after calling the `ren_create_*` function, but after `ren_prepare()`. Memory Managment will be a bit like malloc(). Specifically, it will allocate a larger block of memoy than needed and store the offset and the size for one mesh in the data structure. When creating a new mesh, the data will be put at the end of the buffer, if there's space for it. When destroying a mesh, the data doesn't vanish from the gpu memory. It will just be marked as removable. When there's not enough space, `ren_prepare()` will call a realloc function, which defragmentates the data and moves it into a bigger buffer. More details below. To minimize the data transfer, only the data that changed will be uploaded to GPU.