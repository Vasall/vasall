#ifndef _WORLD_UTILS_H_
#define _WORLD_UTILS_H_

#include <stdio.h>
#include <stdint.h>

/* Calculate the necessary vertices for a terrain */
int calcVertexNum(int vtxnum);

/* Generate the index-buffer for a terrain */
unsigned int *genIndexBuf(int vtxnum, int *idxlen);

/* Store the vertices for the top-section */
int storeTopSection(unsigned int *indices, int rowlen, int vtxnum);

/* Store the vertices for the second-to-last line */
int storeSecondLastLine(unsigned int *indices, int pointer, int rowlen,
		int vtxnum);

/* Store the vertices for the last line */
int storeLastLine(unsigned int *indices, int pointer, int rowlen, 
		int vtxnum);

/* Store the vertices for a single default quad */
int storeQuad(int tl, int tr, int bl, int br, unsigned int *indices, 
		int ptr, int8_t flg);

/* Store the vertices for a single quad in the last row */
int storeLastRowQuad(int tl, int tr, int bl, int br, unsigned int *indices, 
		int ptr, int8_t flg);

/* Store the vertices of the left triangle of a quad  */
int storeLeftTriangle(int tl, int tr, int bl, int br, unsigned int *indices, 
		int ptr, int8_t flg);

#endif
