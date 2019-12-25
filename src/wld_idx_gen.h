#ifndef _WORLD_INDEXGEN_H_
#define _WORLD_INDEXGEN_H_

#include <stdio.h>
#include <stdint.h>

int calculateVertexCount(int vtxnum);
uint32_t *generateIndexBuffer(int vtxnum, int *idxlen);
int storeTopSection(uint32_t *indices, int rowlen, int vtxnum);
int storeSecondLastLine(uint32_t *indices, int pointer, int rowlen, int vtxnum);
int storeLastLine(uint32_t *indices, int pointer, int rowlen, int vtxnum);
int storeQuad(int topLeft, int topRight, int bottomLeft, int bottomRight, 
		uint32_t *indices, int pointer, int8_t rightHanded);
int storeLastRowQuad(int topLeft, int topRight, int bottomLeft, int bottomRight, 
		uint32_t *indices, int pointer, int8_t rightHanded);
int storeLeftTriangle(int topLeft, int topRight, int bottomLeft, int bottomRight, 
		uint32_t *indices, int pointer, int8_t rightHanded);

#endif
