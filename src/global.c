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
	if(argc) {/* Prevent warning for not using argc */}

	core = calloc(1, sizeof(gloWrapper));
	if(core == NULL) return(-1);

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
 * allocated space.
*/
void gloClose(void)
{
	if(core == NULL) return;

	/* Close the different tables */
	mdlClose();
	shdClose();
	texClose();

	/* Clear the object-array */
	objClose();

	/* Free the core-struct */
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
	char dir[128], rel[256], opt[4];

	XSDL_CombinePath(dir, core->bindir, pth);
	XSDL_CombinePath(rel, dir, "include.reg");

	printf("Load register: %s\n", rel);

	fd = fopen(rel, "r+");
	if(fd == NULL) return(-1);

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
			if(texLoad(key, pth) < 0) {
				goto failed;
			}
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
			if(shdSet(key, vert_pth, frag_pth) < 0) {
				goto failed;
			}
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

			if(mdlLoad(key, obj_pth, tex, shd) < 0) {
				goto failed;
			}
		}

		printf("\n");
	}
				
	fclose(fd);

	return(0);

failed:
	fclose(fd);

	return(-1);
}
