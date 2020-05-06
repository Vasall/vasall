#ifndef _WORLD_UTILS_H
#define _WORLD_UTILS_H

#include "sdl.h"

#include <stdio.h>
#include <stdint.h>

extern int calcVertexNum(int vtxnum);
extern unsigned int *genIndexBuf(int vtxnum, int *idxlen);

extern GLubyte *loadPPM(char* fileName, int8_t pathRelative, int* width, int* height);
extern void loadPPMTexture(char* fileName, int8_t pathRelative, GLuint* textures);
extern float **loadPPMHeightmap(char* fileName, int8_t pathRelative, int terrainSize);
extern float** convertImageToHeightmap(GLubyte* image, int heightmapRadius);

#endif
