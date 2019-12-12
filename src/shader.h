#ifndef _SHADER_H_
#define _SHADER_H_

#include <stdint.h>

/*
 * A single vertex containing three
 * float-values representing the 
 * position. (12 bytes)
 */
typedef struct Vertex {
	float x;
	float y;
	float z;
} Vertex;

/* 
 * This is a struct containing 
 * four float-values representing
 * an rgb-color with an optional
 * alpha-value. 
*/
typedef struct Color {
	float r;
	float g;
	float b;
	float a;
} Color;


typedef struct Shader {
	/*
	 * The shader-program.
	 */
	uint32_t id;

	/*
	 * The vertex-shader.
	 */
	uint32_t vshdr;

	/*
	 * The fragment-shader.
	 */
	uint32_t fshdr;
} Shader;

Color colFromRGB(int r, int g, int b);
Color colFromRGBA(int r, int g, int b, int a);

/* Create a new shader program */
Shader *shdCreate(char *vtx_shd, char *frg_shd);

#endif
