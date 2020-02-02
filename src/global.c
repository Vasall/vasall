#include "global.h"

/* Redefine external variables */
uint8_t zero = 0;
uint8_t one = 1;
gloWrapper *core = NULL;
char gloErrBuf[256];

/* 
 * Set a new error-message.
 *
 * @err: The string to set as the new
 * 	error message
*/
void gloSetError(char *err)
{	
	strcpy(gloErrBuf, err);
}

/* 
 * Get the most recent error-message.
 *
 * Returns: The most recent error-message
*/
char *gloGetError(void)
{
	return(gloErrBuf);
}


/*
 * Initialize the global-wrapper and setup the
 * core instances and prepare the attributes.
 *
 * @argc: The number of arguments passed to main
 * @argv: The argument-buffer passed to main
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
*/
int gloInit(int argc, char **argv)
{
	if(argc) {/* Prevent warning for not using argc */ }

	core = calloc(1, sizeof(gloWrapper));
	if(core == NULL) {
		gloSetError("Failed to create global wrapper");
		return(-1);
	}

	/* Just to be save */
	core->procevt = NULL;
	core->update = NULL;
	core->render = NULL;

	/* Initialize the caches */
	if(texInit() < 0) return(-1);
	if(shdInit() < 0) return(-1);
	if(mdlInit() < 0) return(-1);

	/* Initialize the object-array */
	if(objInit() < 0) return(-1);

	/* Get the absolute path to the binary-directory */	
	core->bindir = XSDL_GetBinDir(argv[0]);

	return(0);
}

/*
 * Destroy a global-wrapper and free the
 * allocated space. Note that this function
 * will just skip everything if the ptr is
 * NULL.
 *
 * @ptr: The pointer to the wrapper
*/
void gloClose(void)
{
	if(core == NULL) return;

	mdlClose();

	free(core);
}

/* 
 * Read the include-register and import all necessary
 * resources, which should be defined in the file.
 *
 * @pth: The path to the include-register
 *
 * Returns: Either 0 on success or -1
 * 	if an error occurred
 */
int gloLoad(char *pth)
{
	FILE *fd;
	char dir[128];
	char rel[256];
	char opt[4];

	XSDL_CombinePath(dir, core->bindir, pth);
	XSDL_CombinePath(rel, dir, "include.reg");

	printf("Load register: %s\n", rel);

	fd = fopen(rel, "r+");
	if(fd == NULL) return(-1);

	while(fscanf(fd, "%s", opt) != EOF) {
		printf("%s > ", opt);

		/* Load a texture */
		if(strcmp(opt, "tex") == 0) {
			char key[5], pth[128];

			fscanf(fd, "%s", key);
			printf("[%s] : ", key);

			fscanf(fd, "%s", pth);
			printf("%s", pth);
			XSDL_CombinePath(pth, dir, pth);

			if(texLoad(key, pth) < 0) {
				return(-1);
			}
		}

		/* Load a shader */
		else if(strcmp(opt, "shd") == 0) {
			char key[5], vert_pth[256], frag_pth[256], new_vert[256], new_frag[256];

			/* Read the name of the shader */
			fscanf(fd, "%s", key);
			printf("[%s] : ", key);

			/* Read the path to the vertex shader file */
			fscanf(fd, "%s", vert_pth);
			printf("%s - ", vert_pth);
			XSDL_CombinePath(new_vert, dir, vert_pth);

			/* Read the path to the fragment shader file */
			fscanf(fd, "%s", frag_pth);
			printf("%s ", frag_pth);
			XSDL_CombinePath(new_frag, dir, frag_pth);

			/* Load the shader from the files */
			if(shdSet(key, new_vert, new_frag) < 0) {
				return(-1);
			}
		}

		/* Load a model */
		else if(strcmp(opt, "mdl") == 0) {
			char tmp[64], key[5], obj_pth[256], tex[5], shd[5];
			struct model *mdl;

			/* Read the name of the model */
			fscanf(fd, "%s", key);
			printf("[%s] : ", key);

			/* Read the path to the obj-file */
			fscanf(fd, "%s", tmp);
			printf("%s - ", tmp);
			XSDL_CombinePath(obj_pth, dir, tmp);

			/* Read the name of the texture */
			fscanf(fd, "%s", tex);
			printf("%s - ", tex);

			/* Read the name of the shader */
			fscanf(fd, "%s", shd);
			printf("%s", shd);

			if(mdlLoad(key, obj_pth, tex, shd) < 0) {
				return(-1);
			}
		}

		printf("\n");	
	}
				
	fclose(fd);

	return(0);
}
