#include "render_engine.h"

struct ren_wrapper {
	int vulkan;
};

static struct ren_wrapper ren;

extern int ren_init(SDL_Window *window)
{
	if(vk_init(window) < 0) {
		ren.vulkan = 0;
		if(gl_init(window) < 0) {
			return -1;
		}
		return 0;
	}
	ren.vulkan = 1;
	return 0;
}

extern int ren_destroy(void)
{
	int res;

	if(ren.vulkan)
		res = vk_destroy();
	else
		res = gl_destroy();

	return res;

}

extern int ren_resize(int w, int h)
{
	int res;

	if(ren.vulkan)
		res = vk_resize();
	else
		res = gl_resize(w, h);

	return res;
}

extern int ren_create_shader(char *vs, char *fs, uint32_t *prog,
		struct vk_pipeline *pipeline, int num, char **vars)
{
	int i, res;
	enum vk_in_attr in_attr = 0;
	char *vk_vs, *vk_fs;

	for(i = 0; i < num; i++) {
		if(strcmp(vars[i], "vtxPos") == 0 || strcmp(vars[i], "pos") == 0 ) {
			in_attr |= IN_ATTR_POS;
		}
		if(strcmp(vars[i], "vtxTex") == 0 || strcmp(vars[i], "tex") == 0 ) {
			in_attr |= IN_ATTR_TEX;
		}
		if(strcmp(vars[i], "vtxNrm") == 0) {
			in_attr |= IN_ATTR_NRM;
		}
		if(strcmp(vars[i], "vtxJnt") == 0) {
			in_attr |= IN_ATTR_JNT;
		}
		if(strcmp(vars[i], "vtxWgt") == 0) {
			in_attr |= IN_ATTR_WGT;
		}
	}

	vk_vs = malloc(strlen(vs)+5);
	vk_fs = malloc(strlen(fs)+5);
	strcpy(vk_vs, vs);
	strcpy(vk_fs, fs);
	vk_vs[strlen(vs)+0] = '.';
	vk_vs[strlen(vs)+1] = 's';
	vk_vs[strlen(vs)+2] = 'p';
	vk_vs[strlen(vs)+3] = 'v';
	vk_vs[strlen(vs)+4] = '\0';
	vk_fs[strlen(fs)+0] = '.';
	vk_fs[strlen(fs)+1] = 's';
	vk_fs[strlen(fs)+2] = 'p';
	vk_fs[strlen(fs)+3] = 'v';
	vk_fs[strlen(fs)+4] = '\0';

	if(ren.vulkan)
		res = vk_create_pipeline(vk_vs, vk_fs, in_attr, pipeline);
	else
		res = gl_create_program(vs, fs, prog, num, vars);

	free(vk_vs);
	free(vk_fs);

	return res;
}

extern int ren_destroy_shader(uint32_t prog, struct vk_pipeline pipeline)
{
	int res;

	if(ren.vulkan)
		res = vk_destroy_pipeline(pipeline);
	else
		res = gl_delete_program(prog);

	return res;
}

extern int ren_create_texture(char *pth, uint32_t *hdl,
			struct vk_texture *texture)
{
	int res;

	if(ren.vulkan)
		res = vk_create_texture(pth, texture);
	else
		res = gl_create_texture(pth, hdl);

	return res;
}

extern int ren_destroy_texture(uint32_t hdl, struct vk_texture texture)
{
	int res;

	if(ren.vulkan)
		res = vk_destroy_texture(texture);
	else
		res = gl_destroy_texture(hdl);

	return res;
}

extern int ren_create_model_data(struct vk_pipeline pipeline, uint32_t *vao,
				VkDescriptorSet *set)
{
	int res;

	if(ren.vulkan)
		res = vk_create_constant_data(pipeline, set);
	else
		res = gl_create_vao(vao);

	return res;
}

extern int ren_destroy_model_data(uint32_t vao, VkDescriptorSet set)
{
	int res;

	if(ren.vulkan)
		res = vk_destroy_constant_data(set);
	else
		res = gl_destroy_vao(vao);

	return res;
}

extern int ren_create_buffer(uint32_t vao, int type, size_t size, char *buf,
				uint32_t *bo, struct vk_buffer *buffer)
{
	VkBufferUsageFlags usage;
	int res = 0;

	if(type == GL_ARRAY_BUFFER) {
		usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	} else if(type == GL_ELEMENT_ARRAY_BUFFER) {
		usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	} else if(type == GL_UNIFORM_BUFFER){
		usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	} else {
		usage = type;
	}

	if(ren.vulkan) {
		if(vk_create_buffer(size, usage, 1, buffer) < 0)
			return -1;
		if(buf)
			res = vk_copy_data_to_buffer(buf, *buffer);
	} else
		res = gl_create_buffer(vao, type, size, buf, bo);

	return res;
}

extern int ren_destroy_buffer(uint32_t bo, struct vk_buffer buffer)
{
	int res;

	if(ren.vulkan)
		res = vk_destroy_buffer(buffer);
	else
		res = gl_destroy_buffer(bo);

	return res;
}

extern int ren_set_model_data(uint32_t vao, uint32_t vbo, int stride, int rig,
	VkDescriptorSet set, struct vk_buffer uniform_buffer,
	struct vk_texture texture)
{
	int res;

	if(ren.vulkan) {
		if(vk_set_uniform_buffer(uniform_buffer, set) < 0)
			return -1;
		res = vk_set_texture(texture, set);
	} else {
		res = gl_set_input_attr(vao, vbo, stride, rig);
	}

	return res;
}

extern int ren_start(void)
{
	int res;

	if(ren.vulkan)
		res = vk_render_start();
	else
		res = gl_render_start();

	return res;
}

extern int ren_set_shader(uint32_t prog, int attr, struct vk_pipeline pipeline)
{
	int res;

	if(ren.vulkan)
		res = vk_render_set_pipeline(pipeline);
	else
		res = gl_render_set_program(prog, attr);

	return res;
}

extern int ren_set_vertices(uint32_t vao, struct vk_buffer vtx_buffer,
			struct vk_buffer idx_buffer)
{
	int res;

	if(ren.vulkan) {
		if(vk_render_set_vertex_buffer(vtx_buffer) < 0)
			return -1;
		res = vk_render_set_index_buffer(idx_buffer);
	} else
		res = gl_render_set_vao(vao);

	return res;
}

extern int ren_set_render_model_data(unsigned int uni_buf,
			struct uni_buffer uni, uint32_t hdl, struct vk_pipeline pipeline,
			struct vk_buffer vk_uni_buf, VkDescriptorSet set)
{
	int res;

	if(ren.vulkan) {
		if(vk_copy_data_to_buffer(&uni, vk_uni_buf) < 0)
			return -1;
		res = vk_render_set_constant_data(pipeline, set);
	} else {
		if(gl_render_set_uniform_buffer(uni_buf, uni) < 0)
			return -1;
		res = gl_render_set_texture(hdl);
	}

	return res;
}

extern int ren_draw(uint32_t indices)
{
	int res;

	if(ren.vulkan)
		res = vk_render_draw(indices);
	else
		res = gl_render_draw(indices);

	return res;
}

extern int ren_end(SDL_Window *window)
{
	int res;

	if(ren.vulkan)
		res = vk_render_end();
	else
		res = gl_render_end(window);

	return res;
}