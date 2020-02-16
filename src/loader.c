#include "loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

GLubyte* loadPPM(char* fileName, int8_t pathRelative,
	int* width, int* height)
{
	FILE *file;
	char *path, b[100], c;
	int maxColor, imageWidth, imageHeight, size, red, green, blue, i;
	float scaledColor;
	GLubyte *image;

	/* If path is relative to running directory, get absolute path */
	path = malloc(512 * sizeof(char));
	if(path == NULL) {
		printf("Failed to allocate memory.\n");
		goto failed;
	}

	strcpy(path, fileName);
	if (pathRelative) {
		XSDL_CombinePath(path, core->bindir, fileName);
	}

	/* Open the file in read mode */
	file = fopen(path, "r");
	if (file == NULL) {
		printf("Error. File \"%s\" could not be loaded.\n", path);
		goto failed;
	}

	/* Scan everything up to new line */
	fscanf(file,"%[^\n] ", b);

	/* Check if first two characters are not P3. If not, it's not an ASCII PPM file */
	if (b[0]!='P'|| b[1] != '3') {
		printf("%s is not a PPM file!\n", fileName);
		goto failed;
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

	*width = imageWidth;
	*height = imageHeight;

	return (image);

failed:
	printf("Failed to load PPM-file: %s\n", path);
	exit(0);
}

float **loadPPMHeightmap(char* fileName, int8_t pathRelative, int terrainSize)
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

void loadPPMTexture(char* fileName, int8_t pathRelative, GLuint* textures)
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
