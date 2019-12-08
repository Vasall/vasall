#ifndef VASALL_LOADER_H
#define VASALL_LOADER_H

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#endif

GLubyte* loadPPM(char* fileName, int8_t pathRelative, int* width, int* height);
void loadPPMTexture(char* fileName, int8_t pathRelative, GLuint* textures);
float **loadPPMHeightmap(char* fileName, int8_t pathRelative, int terrainSize);
float** convertImageToHeightmap(GLubyte* image, int heightmapRadius);

#endif
