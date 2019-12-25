#include <stdio.h>
#include <stdlib.h>

#include "world_indexgen.h"
#include "stdutil.h"

/* 
 * Generate an array containing indices for all
 * buffers.
 */
unsigned int *genIdxBuf(int num, int *len)
{
	int idxc = ((num - 1) * (num - 1)) * 6;
	int ptr, col, row;
	unsigned int *idxbuf = malloc(idxc * sizeof(unsigned int));

	*len = idxc;

	ptr = 0;
	for(col = 0; col < num - 1; col++) {
		for(row = 0; row < num - 1; row++) {
			uint32_t topLeft = twodim(col, row, num);
			uint32_t topRight = twodim(col + 1, row, num);
			uint32_t bottomLeft = twodim(col, row + 1, num);
			uint32_t bottomRight = twodim(col + 1, row + 1, num);

			if(row % 2 == 0) {
				ptr = storeQuad1(idxbuf, ptr, 
						topLeft, topRight,
						bottomLeft, bottomRight,
						col % 2 == 0);
			}
			else {
				ptr = storeQuad2(idxbuf, ptr,
						topLeft, topRight,
						bottomLeft, bottomRight,
						col % 2 == 0);
			}
		}
	}

	return(idxbuf);
}

int storeQuad1(uint32_t *idxbuf, int ptr, uint32_t tl, uint32_t tr, 
		uint32_t bl, uint32_t br, int8_t mixed) {
	idxbuf[ptr++] = tl;
	idxbuf[ptr++] = bl;
	idxbuf[ptr++] = mixed ? tr : br;
	idxbuf[ptr++] = br;
	idxbuf[ptr++] = tr;
	idxbuf[ptr++] = mixed ? bl : tl;
	return(ptr);
}

int storeQuad2(uint32_t *idxbuf, int ptr, uint32_t tl, uint32_t tr, 
		uint32_t bl, uint32_t br, int8_t mixed) {
	idxbuf[ptr++] = tr;
	idxbuf[ptr++] = tl;
	idxbuf[ptr++] = mixed ? br : bl;
	idxbuf[ptr++] = bl;
	idxbuf[ptr++] = br;
	idxbuf[ptr++] = mixed ? tl : tr;
	return (ptr);
}

int calculateVertexCount(int vtxnum) {
	int bottom2Rows = 2 * vtxnum;
	int remainingRowCount = vtxnum - 2;
	int topCount = remainingRowCount * (vtxnum - 1) * 2;
	return(topCount + bottom2Rows);
}

uint32_t *generateIndexBuffer(int vtxnum, int *idxlen) {
	uint32_t size = ((vtxnum - 1) * (vtxnum - 1) * 6);
	uint32_t *indices = malloc(size * sizeof(uint32_t));
        int rowlen = (vtxnum - 1) * 2;
        int ptr = storeTopSection(indices, rowlen, vtxnum);
	*idxlen = size;
        ptr = storeSecondLastLine(indices, ptr, rowlen, vtxnum);
        ptr = storeLastLine(indices, ptr, rowlen, vtxnum);
        return (indices);
}
 
int storeTopSection(uint32_t *indices, int rowlen, int vtxnum) {
        int ptr = 0, row, col;
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
        return(ptr);
}

int storeSecondLastLine(uint32_t *indices, int ptr, int rowlen, int vtxnum) {
        int col, row = vtxnum - 3;
        for (col = 0; col < vtxnum - 1; col++) {
            int topLeft = (row * rowlen) + (col * 2);
            int topRight = topLeft + 1;
            int bottomLeft = (topLeft + rowlen) - col;
            int bottomRight = bottomLeft + 1;
            ptr = storeQuad(topLeft, topRight, bottomLeft, 
			    bottomRight, indices, ptr, 
			    ((col % 2) != (row % 2)));
        }
        return(ptr);
    }
 
int storeLastLine(uint32_t *indices, int ptr, int rowlen, int vtxnum) {
        int col, row = vtxnum - 2;
        for (col = 0; col < vtxnum - 1; col++) {
            int topLeft = (row * rowlen) + col;
            int topRight = topLeft + 1;
            int bottomLeft = (topLeft + vtxnum);
            int bottomRight = bottomLeft + 1;
            ptr = storeLastRowQuad(topLeft, topRight, bottomLeft, 
			    bottomRight, indices, ptr, 
			    ((col % 2) != (row % 2)));
        }
        return(ptr);
    }
 
int storeQuad(int topLeft, int topRight, int bottomLeft, int bottomRight, 
		uint32_t *indices, int ptr, int8_t rightHanded) {
        ptr = storeLeftTriangle(topLeft, topRight, bottomLeft, 
			bottomRight, indices, ptr, rightHanded);
        indices[ptr++] = topRight;
        indices[ptr++] = rightHanded ? topLeft : bottomLeft;
        indices[ptr++] = bottomRight;
        return(ptr);
}
     
int storeLastRowQuad(int topLeft, int topRight, int bottomLeft, int bottomRight, 
		uint32_t *indices, int ptr, int8_t rightHanded) {
        ptr = storeLeftTriangle(topLeft, topRight, bottomLeft, 
			bottomRight, indices, ptr, rightHanded);
        indices[ptr++] = bottomRight;
        indices[ptr++] = topRight;
        indices[ptr++] = rightHanded ? topLeft : bottomLeft;
        return(ptr);
}
     
int storeLeftTriangle(int topLeft, int topRight, int bottomLeft, int bottomRight, 
		uint32_t *indices, int ptr, int8_t rightHanded) {
        indices[ptr++] = topLeft;
        indices[ptr++] = bottomLeft;
        indices[ptr++] = rightHanded ? bottomRight : topRight;
        return(ptr);
}
