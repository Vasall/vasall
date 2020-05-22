#ifndef _ASSET_H
#define _ASSET_H

#include "shader.h"
#include "texture.h"
#include "text.h"


/*
 * This function works as a wrapper function to initialize all asset-tables,
 * ie shaders, textures and fonts. 
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int ast_init(void);


/*
 * Close the asset-tables and free the allocated memory.
 */
extern void ast_close(void);

#endif
