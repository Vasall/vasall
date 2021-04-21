#include "render_engine.h"


/* Redefine global render-wrapper */
struct ren_wrapper g_ren;


extern int ren_init(SDL_Window *window, char mode)
{
	if(mode == 0) {
		g_ren.mode = REN_MODE_VULKAN;
		if(vk_init(window) < 0) {
			return -1;
		}

	}
	else if(mode == 1) {
		g_ren.mode = REN_MODE_OPENGL;
		
		if(gl_init(window) < 0) {
			return -1;
		}
	}
	
	return 0;
}


extern void ren_destroy(void)
{
	if(g_ren.mode == REN_MODE_VULKAN) {
		vk_destroy();
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_destroy();
	}
}


extern int ren_resize(int w, int h)
{
	int res;

	if(g_ren.mode == REN_MODE_VULKAN) {
		res = vk_resize();
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_resize(w, h);
		res = 0;
	}

	return res;
}


extern int ren_create_shader(char *vs, char *fs, uint32_t *prog,
                             struct vk_pipeline *pipeline, int num, char **vars,
							 enum mdl_type type)
{
	int i, res;
	enum vk_in_attr in_attr = 0;
	char *vk_vs, *vk_fs;
	size_t vs_len, fs_len;

	for(i = 0; i < num; i++) {
		if(strcmp(vars[i], "vtxPos") == 0 || strcmp(vars[i], "pos")
		   == 0 ) {
			in_attr |= IN_ATTR_POS;
			continue;
		}
		if(strcmp(vars[i], "vtxTex") == 0 || strcmp(vars[i], "tex")
		   == 0 ) {
			in_attr |= IN_ATTR_TEX;
			continue;
		}
		if(strcmp(vars[i], "vtxNrm") == 0) {
			in_attr |= IN_ATTR_NRM;
			continue;
		}
		if(strcmp(vars[i], "vtxJnt") == 0) {
			in_attr |= IN_ATTR_JNT;
			continue;
		}
		if(strcmp(vars[i], "vtxWgt") == 0) {
			in_attr |= IN_ATTR_WGT;
			continue;
		}
	}

	vs_len = strlen(vs);
	fs_len = strlen(fs);
	vk_vs = malloc(vs_len+5);
	vk_fs = malloc(fs_len+5);
	strncpy(vk_vs, vs, vs_len);
	strncpy(vk_fs, fs, fs_len);
	vk_vs[vs_len+0] = '.';
	vk_vs[vs_len+1] = 's';
	vk_vs[vs_len+2] = 'p';
	vk_vs[vs_len+3] = 'v';
	vk_vs[vs_len+4] = '\0';
	vk_fs[fs_len+0] = '.';
	vk_fs[fs_len+1] = 's';
	vk_fs[fs_len+2] = 'p';
	vk_fs[fs_len+3] = 'v';
	vk_fs[fs_len+4] = '\0';

	if(g_ren.mode == REN_MODE_VULKAN) {
		res = vk_create_pipeline(vk_vs, vk_fs, in_attr, type, pipeline);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		res = gl_create_program(vs, fs, prog, num, vars);
	}

	free(vk_vs);
	free(vk_fs);

	return res;
}


extern void ren_destroy_shader(uint32_t prog, struct vk_pipeline pipeline)
{
	if(g_ren.mode == REN_MODE_VULKAN) {
		vk_destroy_pipeline(pipeline);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_delete_program(prog);
	}
}


extern int ren_create_texture(char *pth, uint32_t *hdl,
                              struct vk_texture *texture)
{
	int res;

	if(g_ren.mode == REN_MODE_VULKAN) {
		res = vk_create_texture(pth, texture);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		res = gl_create_texture(pth, hdl);
	}

	return res;
}


extern void ren_destroy_texture(uint32_t hdl, struct vk_texture texture)
{
	if(g_ren.mode == REN_MODE_VULKAN) {
		vk_destroy_texture(texture);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_destroy_texture(hdl);
	}
}


extern int ren_create_skybox(char *pths[6], uint32_t *hdl,
			     struct vk_texture *skybox)
{
	int res;

	if(g_ren.mode == REN_MODE_VULKAN) {
		res = vk_create_skybox(pths, skybox);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		res = gl_create_skybox(pths, hdl);
	}

	return res;
}


extern int ren_create_model_data(struct vk_pipeline pipeline, uint32_t *vao,
                                 VkDescriptorSet *set)
{
	int res;

	if(g_ren.mode == REN_MODE_VULKAN) {
		res = vk_create_constant_data(pipeline, set);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_create_vao(vao);
		res = 0;
	}

	return res;
}


extern int ren_destroy_model_data(uint32_t vao, VkDescriptorSet set)
{
	int res;

	if(g_ren.mode == REN_MODE_VULKAN) {
		res = vk_destroy_constant_data(set);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_destroy_vao(vao);
		res = 0;
	}

	return res;
}


extern int ren_create_buffer(uint32_t vao, int type, size_t size, char *buf,
                             uint32_t *bo, struct vk_buffer *buffer)
{
	VkBufferUsageFlags usage;
	int res = 0;

	if(type == GL_ARRAY_BUFFER) {
		usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}
	else if(type == GL_ELEMENT_ARRAY_BUFFER) {
		usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}
	else if(type == GL_UNIFORM_BUFFER){
		usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	else {
		usage = type;
	}

	if(g_ren.mode == REN_MODE_VULKAN) {
		if(vk_create_buffer(size, usage, 1, buffer) < 0)
			return -1;
		
		if(buf) {
			vk_copy_data_to_buffer(buf, *buffer);
			res = 0;
		}
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		res = gl_create_buffer(vao, type, size, buf, bo);
	}

	return res;
}


extern void ren_destroy_buffer(uint32_t bo, struct vk_buffer buffer)
{
	if(g_ren.mode == REN_MODE_VULKAN) {
		vk_destroy_buffer(buffer);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_destroy_buffer(bo);
	}
}


extern int ren_set_model_data(uint32_t vao, uint32_t vbo, int stride, int rig,
                              VkDescriptorSet set,
                              struct vk_buffer uniform_buffer,
                              struct vk_texture texture)
{
	int res;

	if(g_ren.mode == REN_MODE_VULKAN) {
		if(vk_set_uniform_buffer(uniform_buffer, set) < 0)
			return -1;
		
		res = vk_set_texture(texture, set);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_set_input_attr(vao, vbo, stride, rig);
		res = 0;
	}

	return res;
}


extern int ren_start(void)
{
	int res;

	if(g_ren.mode == REN_MODE_VULKAN) {
		res = vk_render_start();
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_render_start();
		res = 0;
	}

	return res;
}


extern int ren_set_shader(uint32_t prog, int attr, struct vk_pipeline pipeline)
{
	int res;

	if(g_ren.mode == REN_MODE_VULKAN) {
		vk_render_set_pipeline(pipeline);
		res = 0;
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		res = gl_render_set_program(prog, attr);
	}

	return res;
}


extern void ren_set_vertices(uint32_t vao, struct vk_buffer vtx_buffer,
                            struct vk_buffer idx_buffer)
{
	if(g_ren.mode == REN_MODE_VULKAN) {
		vk_render_set_vertex_buffer(vtx_buffer);
		vk_render_set_index_buffer(idx_buffer);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_render_set_vao(vao);
	}
}


extern void ren_set_render_model_data(unsigned int uni_buf,
                                     struct uni_buffer uni, uint32_t hdl,
                                     struct vk_pipeline pipeline,
                                     struct vk_buffer vk_uni_buf,
                                     VkDescriptorSet set, enum mdl_type type)
{
	if(g_ren.mode == REN_MODE_VULKAN) {
		vk_copy_data_to_buffer(&uni, vk_uni_buf);
		vk_render_set_constant_data(pipeline, set);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_render_set_uniform_buffer(uni_buf, uni);
		gl_render_set_texture(hdl, type);
	}
}


extern void ren_draw(uint32_t indices, enum mdl_type type)
{
	if(g_ren.mode == REN_MODE_VULKAN) {
		vk_render_draw(indices);
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_render_draw(indices, type);
	}
}


extern int ren_end(SDL_Window *window)
{
	int res;

	if(g_ren.mode == REN_MODE_VULKAN) {
		res = vk_render_end();
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_render_end(window);
		res = 0;
	}

	return res;
}


extern int ren_print_info(void)
{
	int res;
	if(g_ren.mode == REN_MODE_VULKAN) {
		res = vk_print_info();
	}
	else if(g_ren.mode == REN_MODE_OPENGL) {
		gl_print_info();
		res = 0;
	}

	return res;
}
