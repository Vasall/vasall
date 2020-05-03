#include "core.h"
#include "render.h"
#include "client.h"

/* Redefine external variables */
uint8_t zero = 0;
uint8_t one = 1;
struct core_wrapper *core = NULL;
char glo_err_buf[256];

V_API void glo_set_err(char *err)
{	
	strcpy(glo_err_buf, err);
}

V_API char *glo_get_err(void)
{
	return glo_err_buf;
}

V_API int core_init(int argc, char **argv)
{
	if(argc) {/* Prevent warning for not using argc */}

	if(!(core = malloc(sizeof(struct core_wrapper))))
		return -1;

	memset(core, 0, sizeof(struct core_wrapper));

	/* Just to be save */
	core->procevt = NULL;
	core->update = NULL;
	core->render = NULL;

	if(cli_init("::1", 4242) < 0)
		return -1;

	if(mdl_init() < 0)
		return -1;

	if(obj_init() < 0)
		return -1;

	/* Get the absolute path to the binary-directory */	
	core->bindir = XSDL_GetBinDir(argv[0]);

	return 0;
}

V_API void core_close(void)
{
	if(!core)
		return;

	cli_close();
	
	obj_close();
	mdl_close();

	free(core);
}

V_API int core_load(char *pth)
{
	FILE *fd = NULL;
	char dir[256], rel[512], opt[512];

	XSDL_CombinePath(dir, core->bindir, pth);
	XSDL_CombinePath(rel, dir, "include.reg");

	printf("Load register: %s\n", rel);

	if(!(fd = fopen(rel, "r")))
		return -1;
	
	while(1) {
		printf("%p\n", fd);
		printf("before\n");
		if(fscanf(fd, "%99s", opt) == EOF) {
			printf("break\n");
			break;
		}
		printf("after\n");

		printf("%s > ", opt);

		/* Load a texture and push it into the texture-table */
		if(strcmp(opt, "tex") == 0) {
			char key[5], pth[128];

			/* Read the key of the texture */
			fscanf(fd, "%s", key);
			printf("[%s] : ", key);

			/* Read the path to the texture */
			fscanf(fd, "%s", pth);
			printf("%s", pth);
			XSDL_CombinePath(pth, dir, pth);

			/* Load the pixel-data */
			printf("Load texture\n");
			if(tex_load_png(key, pth) < 0)
				goto failed;
		}

		/* Load a shader and push it into the shader-table */
		else if(strcmp(opt, "shd") == 0) {
			char key[5], vert_pth[256], frag_pth[256];

			/* Read the key of the model */
			fscanf(fd, "%s", key);
			printf("[%s] : ", key);

			/* Read the path to the vertex-shader */
			fscanf(fd, "%s", vert_pth);
			printf("%s - ", vert_pth);
			XSDL_CombinePath(vert_pth, dir, vert_pth);

			/* Read the path to the fragment-shader */
			fscanf(fd, "%s", frag_pth);
			printf("%s ", frag_pth);
			XSDL_CombinePath(frag_pth, dir, frag_pth);

			/* Load the shader from the files */
			printf("Set shader\n");
			if(shd_set(key, vert_pth, frag_pth) < 0)
				goto failed;
		}

		/* Load a model and push it into model-table */
		else if(strcmp(opt, "mdl") == 0) {
			char tmp[64], key[5], obj_pth[256], tex[5], shd[5];
			short tex_slot;
			short shd_slot;

			/* Read the name of the model */
			fscanf(fd, "%s", key);
			printf("[%s] : ", key);

			/* Read the path to the obj-file */
			fscanf(fd, "%s", tmp);
			printf("%s - ", tmp);
			XSDL_CombinePath(obj_pth, dir, tmp);

			/* Read the path to the tex-file */
			fscanf(fd, "%s", tex);
			printf("%s - ", tex);

			tex_slot = tex_get(tex);
				
			/* Read the path to the shader-file */
			fscanf(fd, "%s", shd);
			printf("%s", shd);

			shd_slot = shd_get(shd);

			printf("Model\n");
			if(mdl_load(key, obj_pth, tex_slot, shd_slot) < 0)
				goto failed;
		}

		printf("\n");
	}
	
	printf("adda\n");
	fclose(fd);
	return 0;

failed:
	fclose(fd);
	return -1;
}

V_API void core_update(void)
{
	/* Update the client */
	cli_update();

	/* Update the userinterface */
	XSDL_UpdateUIContext(window->ui_ctx);

	/* Run specified update-function */
	if(core->update)
		core->update();
}

V_API void core_render(void)
{
	/* Clear the screen */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Run current render-function */
	if(core->render)
		core->render();

	/* Render the current userinterface */
	XSDL_Render(window->ui_ctx);

	/* Render the buffer on the screen */
	XSDL_GL_SwapWindow(window->win);
}
