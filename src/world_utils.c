#include "world_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

V_INTERN int storeTopSection(unsigned int *indices, int rowlen, int vtxnum);
V_INTERN int storeSecondLastLine(unsigned int *indices, int ptr, int rowlen, 
		int vtxnum);
V_INTERN int storeLastLine(unsigned int *indices, int ptr, int rowlen, 
		int vtxnum);
V_INTERN int storeQuad(int tl, int tr, int bl, int br, unsigned int *indices,
		int ptr, int8_t flg);
V_INTERN int storeLastRowQuad(int tl, int tr, int bl, int br,
		unsigned int *indices, int ptr, int8_t flg);
V_INTERN int storeLeftTriangle(int tl, int tr, int bl, int br, 
		unsigned int *indices, int ptr, int8_t flg);

/*
 * Fill the vertices in the lines up to the second-to-last. From there special
 * conditions apply.
 *
 * @indices: The index-buffer
 * @rowlen: The length of a single row
 * @vtxnum: The side-length of the square-terrain
 *
 * Returns: The current position in the index-buffer
 */
V_INTERN int storeTopSection(unsigned int *indices, int rowlen, int vtxnum)
{
	int ptr = 0;
	int row;
	int col;
	
	for (row = 0; row < vtxnum - 3; row++) {
		for (col = 0; col < vtxnum - 1; col++) {
			int topLeft = (row * rowlen) + (col * 2);
			int topRight = topLeft + 1;
			int bottomLeft = topLeft + rowlen;
			int bottomRight = bottomLeft + 1;
			ptr = storeQuad(topLeft, topRight, bottomLeft, 
					bottomRight, indices, ptr, 
					col % 2 != row % 2);
		}
	}
	
	return ptr;
}

/*
 * Store the vertices for the second-to-last line. 
 *
 * @indices: The index-buffer
 * @ptr: The current position in the index-buffer
 * @rowlen: The length of a single row
 * @vtxnum: The side-length of the square-terrain
 *
 * Returns: The updated position in the index-array 
 */
V_INTERN int storeSecondLastLine(unsigned int *indices, int ptr, int rowlen, 
		int vtxnum)
{
	int col;
	int row = vtxnum - 3;
	
	for (col = 0; col < vtxnum - 1; col++) {
		int topLeft = (row * rowlen) + (col * 2);
		int topRight = topLeft + 1;
		int bottomLeft = (topLeft + rowlen) - col;
		int bottomRight = bottomLeft + 1;
		ptr = storeQuad(topLeft, topRight, bottomLeft, 
				bottomRight, indices, ptr, 
				((col % 2) != (row % 2)));
	}

	return ptr;
}

/*
 * Store the vertices for the last line.
 *
 * @indices: The index-buffer
 * @ptr: The current position in the index-buffer
 * @rowlen: The length of a single row
 * @vtxnum: The side-length of the square-terrain
 */
V_INTERN int storeLastLine(unsigned int *indices, int ptr, int rowlen, 
		int vtxnum)
{
	int col;
	int row = vtxnum - 2;
	
	for (col = 0; col < vtxnum - 1; col++) {
		int topLeft = (row * rowlen) + col;
		int topRight = topLeft + 1;
		int bottomLeft = (topLeft + vtxnum);
		int bottomRight = bottomLeft + 1;
		ptr = storeLastRowQuad(topLeft, topRight, bottomLeft, 
				bottomRight, indices, ptr, 
				((col % 2) != (row % 2)));
	}

	return ptr;
}

/*
 * Store the vertices for a single quad. Note that this is a subfunction, that
 * will be called by the generation-function.
 *
 * @tl: The number of the top-left vertex
 * @tr: The number of the top-right vertex
 * @bl: The number of the bottom-left vertex
 * @br: The number of the bottom-right vertex
 * @indices: The index-buffer
 * @ptr: The current position in the index-buffer
 * @flg: Should the top- or bottom-left point be used
 *
 * Returns: The updated position in the index-buffer
 */
V_INTERN int storeQuad(int tl, int tr, int bl, int br, unsigned int *indices,
		int ptr, int8_t flg)
{
	ptr = storeLeftTriangle(tl, tr, bl, br, indices, ptr, flg);
	indices[ptr++] = tr;
	indices[ptr++] = flg ? tl : bl;
	indices[ptr++] = br;

	return ptr;
}

/*
 * Store the vertices for all quads of the last row.
 * 
 * @tl: The number of the top-left vertex
 * @tr: The number of the top-right vertex
 * @bl: The number of the bottom-left vertex
 * @br: The number of the bottom-right vertex
 * @indices: The index-buffer
 * @ptr: The current position in the index-buffer
 * @flg: Should the top- or bottom-left point be used
 *
 * Returns: The updated position in the index-buffer
 */
V_INTERN int storeLastRowQuad(int tl, int tr, int bl, int br,
		unsigned int *indices, int ptr, int8_t flg)
{
	ptr = storeLeftTriangle(tl, tr, bl, br, indices, ptr, flg);
	indices[ptr++] = br;
	indices[ptr++] = tr;
	indices[ptr++] = flg ? tl : bl;
	return ptr;
}

/*
 * Store the vertices for the left triangle of the quad. This can be abstracted
 * as a single function, as it is the same for all quads.
 *
 * @tl: The number of the top-left vertex
 * @tr: The number of the top-right vertex
 * @bl: The number of the bottom-left vertex
 * @br: The number of the bottom-right vertex
 * @indices: The index-buffer
 * @ptr: The current position in the index-buffer
 * @flg: Should the top- or bottom-right point be used
 *
 * Returns: The updated position in the index-buffer
 */
V_INTERN int storeLeftTriangle(int tl, int tr, int bl, int br, 
		unsigned int *indices, int ptr, int8_t flg)
{
	indices[ptr++] = tl;
	indices[ptr++] = bl;
	indices[ptr++] = flg ? br : tr;
	return ptr;
}

V_API int calcVertexNum(int vtxnum)
{
	int bottom2Rows = 2 * vtxnum;
	int remainingRowCount = vtxnum - 2;
	int topCount = remainingRowCount * (vtxnum - 1) * 2;
	return topCount + bottom2Rows;
}

V_API unsigned int *genIndexBuf(int vtxnum, int *idxlen)
{
	unsigned int size = ((vtxnum - 1) * (vtxnum - 1) * 6);
	unsigned int *indices;
	int ptr;
	int rowlen = (vtxnum - 1) * 2;

	*idxlen = size;
	if(!(indices = malloc(size * sizeof(uint32_t))))
		return NULL;
	
	ptr = storeTopSection(indices, rowlen, vtxnum);
	ptr = storeSecondLastLine(indices, ptr, rowlen, vtxnum);
	ptr = storeLastLine(indices, ptr, rowlen, vtxnum);

	return indices;
}
