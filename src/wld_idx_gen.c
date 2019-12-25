#include <stdio.h>
#include <stdlib.h>

#include "wld_idx_gen.h"
#include "stdutil.h"

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
