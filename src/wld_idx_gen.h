#ifndef _WORLD_INDEXGEN_H_
#define _WORLD_INDEXGEN_H_

#include <stdio.h>
#include <stdint.h>

/* Calculate the necessary vertices for a terrain */
int calcVertexNum(int vtxnum);

/* Generate the index-buffer for a terrain */
uint32_t *genIndexBuf(int vtxnum, int *idxlen);

/* Store the vertices for the top-section */
int storeTopSection(uint32_t *indices, int rowlen, int vtxnum);

/* Store the vertices for the second-to-last line */
int storeSecondLastLine(uint32_t *indices, int pointer, int rowlen, int vtxnum);

/* Store the vertices for the last line */
int storeLastLine(uint32_t *indices, int pointer, int rowlen, int vtxnum);

/* Store the vertices for a single default quad */
int storeQuad(int tl, int tr, int bl, int br, uint32_t *indices, 
		int ptr, int8_t flg);

/* Store the vertices for a single quad in the last row */
int storeLastRowQuad(int tl, int tr, int bl, int br, uint32_t *indices, 
		int ptr, int8_t flg);

/* Store the vertices of the left triangle of a quad  */
int storeLeftTriangle(int tl, int tr, int bl, int br, uint32_t *indices, 
		int ptr, int8_t flg);

#endif
