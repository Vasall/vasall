#include "world_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

static int storeTopSection(unsigned int *indices, int rowlen, int vtxnum);
static int storeSecondLastLine(unsigned int *indices, int ptr, int rowlen, 
		int vtxnum);
static int storeLastLine(unsigned int *indices, int ptr, int rowlen, 
		int vtxnum);
static int storeQuad(int tl, int tr, int bl, int br, unsigned int *indices,
		int ptr, int8_t flg);
static int storeLastRowQuad(int tl, int tr, int bl, int br,
		unsigned int *indices, int ptr, int8_t flg);
static int storeLeftTriangle(int tl, int tr, int bl, int br, 
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
static int storeTopSection(unsigned int *indices, int rowlen, int vtxnum)
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
static int storeSecondLastLine(unsigned int *indices, int ptr, int rowlen, 
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
static int storeLastLine(unsigned int *indices, int ptr, int rowlen, 
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
static int storeQuad(int tl, int tr, int bl, int br, unsigned int *indices,
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
static int storeLastRowQuad(int tl, int tr, int bl, int br,
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
static int storeLeftTriangle(int tl, int tr, int bl, int br, 
		unsigned int *indices, int ptr, int8_t flg)
{
	indices[ptr++] = tl;
	indices[ptr++] = bl;
	indices[ptr++] = flg ? br : tr;
	return ptr;
}

extern int calcVertexNum(int vtxnum)
{
	int bottom2Rows = 2 * vtxnum;
	int remainingRowCount = vtxnum - 2;
	int topCount = remainingRowCount * (vtxnum - 1) * 2;
	return topCount + bottom2Rows;
}

extern unsigned int *genIndexBuf(int vtxnum, int *idxlen)
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

extern GLubyte *loadPPM(char* fileName, int8_t pathRelative,
	int* width, int* height)
{
	FILE *file;
	char path[512], b[100], c;
	int maxColor, imageWidth, imageHeight, size, red, green, blue, i;
	float scaledColor;
	GLubyte *image;

	strcpy(path, fileName);

	/* Open the file in read mode */
	if(!(file = fopen(path, "r")))
		return NULL;

	/* Scan everything up to new line */
	fscanf(file,"%[^\n] ", b);

	/* Check if first two characters are not P3. If not, it's not an ASCII PPM file */
	if (b[0]!='P'|| b[1] != '3') {
		fclose(file);
		return NULL;
	}

	/* Read past the file comments (then go back 1 so we don't miss the size) */
	fscanf(file, "%c", &c);
	while(c == '#')	{
		fscanf(file, "%[^\n] ", b);
		fscanf(file, "%c", &c);
	}
	ungetc(c, file);

	/* Read the rows, columns and max colour values */
	fscanf(file, "%d %d %d", &imageWidth, &imageHeight, &maxColor);

	/* Number of pixels is width * height */
	size = imageWidth * imageHeight;

	/* Allocate memory to store 3 GLuints for every pixel */
	image = (GLubyte *)malloc(3 * sizeof(GLuint) * size);

	/* Scale the colour in case maxCol is not 255 */
	scaledColor = 255.0 / maxColor;

	/* Start reading pixel colour data */
	for (i = 0; i < size; i++) {
		fscanf(file,"%d %d %d", &red, &green, &blue );
		image[3 * size - 3 * i - 3] = red * scaledColor;
		image[3 * size - 3 * i - 2] = green * scaledColor;
		image[3 * size - 3 * i - 1] = blue * scaledColor;
	}

	fclose(file);
	*width = imageWidth;
	*height = imageHeight;
	return image;
}

extern float **loadPPMHeightmap(char* fileName, int8_t pathRelative, int terrainSize)
{
	/* Load the image */
	int width, height, i, j;
	GLubyte* img;
	float **heightmapImage;

	img = loadPPM(fileName, pathRelative, &width, &height);
	if(img == NULL) {
		return(NULL);
	}

	/* Check that image is same size as terrain */
	if (width < terrainSize || height < terrainSize) {
		printf("\nError. Image too small.\n");
		exit(0);
	}
	else if (width > terrainSize || height > terrainSize) {
		printf("\nError. Image too large.\n");
		exit(0);
	}

	/* Allocate mem for our resulting image */
	heightmapImage = (float **)malloc(sizeof(float *) * terrainSize);
	if(heightmapImage == NULL) {
		printf("Failed to allocate memory.\n");
		exit(1);
	}
	for (i = 0; i < terrainSize; i++) {
		heightmapImage[i] = (float *)malloc(sizeof(float) * terrainSize);
		if(heightmapImage[i] == NULL) {
			printf("ERROR!!\n");
			exit(0);
		}
	}

	/* Convert image to 2D float array, averaging RGB values */
	for (i = 0; i < terrainSize; i++) {
		for (j = 0; j < terrainSize; j++) {
			int subscript = (int)(3 * (i * terrainSize + j) +
					3 *(i * terrainSize + j) + 1 + 3 *
					(i * terrainSize + j) + 2) / 3.0;
			heightmapImage[i][j] = img[subscript]/255.0;
		}
	}

	return (heightmapImage);
}

extern void loadPPMTexture(char* fileName, int8_t pathRelative, GLuint* textures)
{
	/* Load the image from the file */
	int width, height;
	GLubyte* img;

	img = loadPPM(fileName, pathRelative, &width, &height);

	/* Set texture properties */
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
}
