#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <stdint.h>
#include "vec.h"
#include "hashtable.h"

#define TEX_SLOTS 8

struct texture {
	/* The id of this texture in OpenGL */
	uint32_t id;

	/* The buffer containing the texture */
	uint8_t *buf;

	/* The size of this texture */
	int width;
	int height;
	int size;
};

/* The global texture-table */
extern struct ht_t *tex_table;

/* Initialize the texture-table */
int texInit(void);

/* Close the texture-table */
void texClose(void);

/* Create a new texture */
int texSet(char *key, uint8_t *buf, int w, int h);

/* Load a texture from a file */
int texLoad(char *key, char *pth);

/* Get a texture from the table */
struct texture *texGet(char *key);

/* Delete a texture */
void texDel(char *key);

#endif
