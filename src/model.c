#include <stdlib.h>

#include "model.h"

/* 
 * Create a new model and attached the
 * given attributes to the vao. Then
 * compile the shaders and link them to
 * the shaderprogram. Note that all 
 * parameters are optional, apart from
 * the vertex-array.
 *
 * @vtx: An array containing all vertices
 * @vnum: The length of the vertex-array
 * @col: An array containing all colors
 * @cnum: The length of the color-array
 * @norm: An array containing all normal-vectors
 * @nnum: The length of the normal-vec-array
 * @vtxshd: The relative path to the vertex-shader
 * @frgshd: The relative path to the fragment-shader
 *
 * Returns: A pointer to the new model or
 * 	NULL if an error occurred
*/
/*Model *modCreate(Vec3 pos, Vec3 size, float *vtx, int vnum, Color *col, 
		int cnum, Vertex *norm, int nnum, char *vtxshd, char *frgshd)
{
	
}
*/

/* Render a model */
void modRender(Model *mod);


