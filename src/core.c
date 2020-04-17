#include "core.h"

/* Redefine external variables */
uint8_t zero = 0;
uint8_t one = 1;
struct core_wrapper *core = NULL;
char glo_err_buf[256];

void glo_set_err(char *err)
{	
	strcpy(glo_err_buf, err);
}

char *glo_get_err(void)
{
	return glo_err_buf;
}


int core_init(int argc, char **argv)
{
	if(argc) {/* Prevent warning for not using argc */}

	if(!(core = calloc(1, sizeof(struct core_wrapper))))
		return -1;

	/* Just to be save */
	core->procevt = NULL;
	core->update = NULL;
	core->render = NULL;

	/* Initialize the caches */
	if(tex_init() < 0) 
		return -1;

	if(shd_init() < 0)
		return -1;

	if(mdl_init() < 0)
		return -1;

	/* Initialize the object-array */
	if(obj_init() < 0)
		return -1;

	/* Get the absolute path to the binary-directory */	
	core->bindir = XSDL_GetBinDir(argv[0]);

	return 0;
}

void core_close(void)
{
	if(core)
		return;

	mdl_close();
	shd_close();
	tex_close();

	obj_close();

	free(core);
}

int core_load(char *pth)
{
	FILE *fd;
	char dir[128], rel[256], opt[4];

	XSDL_CombinePath(dir, core->bindir, pth);
	XSDL_CombinePath(rel, dir, "include.reg");

	printf("Load register: %s\n", rel);

	fd = fopen(rel, "r+");
	if(fd == NULL)
		return -1;

	while(fscanf(fd, "%s", opt) != EOF) {
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
			if(tex_load(key, pth) < 0)
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
			if(shd_set(key, vert_pth, frag_pth) < 0)
				goto failed;
		}

		/* Load a model and push it into model-table */
		else if(strcmp(opt, "mdl") == 0) {
			char tmp[64], key[5], obj_pth[256], tex[5], shd[5];

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

			/* Read the path to the shader-file */
			fscanf(fd, "%s", shd);
			printf("%s", shd);

			if(mdl_load(key, obj_pth, tex, shd) < 0)
				goto failed;
		}

		printf("\n");
	}
				
	fclose(fd);
	return 0;

failed:
	fclose(fd);
	return -1;
}
