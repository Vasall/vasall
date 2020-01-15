#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "vec.h"
#include "mat.h"

/* 
 * Set the values of a 3d-vector.
 *
 * @v: A pointer to the vector
 * @x: The new x-value
 * @y: The new y-value
 * @z: The new z-value
*/
void vecSet(Vec3 v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

/* 
 * Copy the values from the second vector 
 * into the first.
 *
 * @v1: The vector to copy into
 * @v2: The vector to copy from
*/
void vecCpy(Vec3 dst, Vec3 src)
{
	memcpy(dst, src, VEC3_SIZE);
}

/* 
 * Add two vectors.
 *
 * @v1: The first vector
 * @v2: The second vector
 * @res: The resulting vector to write to
*/
void vecAdd(Vec3 v1, Vec3 v2, Vec3 res)
{
	res[0] = v1[0] + v2[0];
	res[1] = v1[1] + v2[1];
	res[2] = v1[2] + v2[2];
}

/* 
 * Subtract one vector from another. 
 *
 * @v1: The first vector
 * @v2: The second vector
 * @res: The resulting vector write to
*/
void vecSub(Vec3 v1, Vec3 v2, Vec3 res)
{
	res[0] = v1[0] - v2[0];
	res[1] = v1[1] - v2[1];
	res[2] = v1[2] - v2[2];
}

/* 
 * Scale a vector by a factor.
 *
 * @v: A pointer to the vector to scale
 * @f: The scaling factor
 * @res: The resulting vector to write to
*/
void vecScl(Vec3 v, float f, Vec3 res)
{
	res[0] = v[0] * f;
	res[1] = v[1] * f;
	res[2] = v[2] * f;
}

void vecRotX(Vec3 v, float angle, Vec3 res) {
	Mat3 rmat;
	mat3Idt(rmat);
	rmat[];
	rmat[];
	rmat[];
	rmat[];
}

void vecRotY(Vec3 v, float angle, Vec3 res) {
	Mat3 rmat;

}

void vecRotZ(Vec3 v, float angle, Vec3 res) {
	Mat3 rmat;
}

/*
 * Divide the vector by the given
 * factor. This function is the
 * inverse to the scale-procedure.
 *
 * @v: The vector to scale
 * @f: The scaling factor
 * @res: The resulting vector to write to
 */
void vecInvScl(Vec3 v, float f, Vec3 res)
{
	res[0] = v[0] / f;
	res[1] = v[1] / f;
	res[2] = v[2] / f;
}


/* 
 * Get the magnitude of a vector.
 *
 * @v: The vector to calculate the magnitude for
 *
 * Returns: The length of the vector
*/
float vecMag(Vec3 v)
{
	double len = (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
	return((float)sqrt(len));
}

/* 
 * Normalize a vector.
 *
 * @v: The vector to normalize
 * @res: The resulting vector to write to
*/
void vecNrm(Vec3 v, Vec3 res)
{
	float len = vecMag(v);
	res[0] = v[0] / len;
	res[1] = v[1] / len;
	res[2] = v[2] / len;
}

/*
 * Calculate the cross-product of 
 * two vectors.
 *
 * @v1: The first vector
 * @v2: The second vector
 * @res: The resulting vector to write to
 */
void vecCross(Vec3 v1, Vec3 v2, Vec3 res)
{
	res[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	res[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	res[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

/*
 * Print a vector in the terminal.
 *
 * @v: The vector to display
 */
void vecPrint(Vec3 v)
{
	printf("%.2f/%.2f/%.2f", v[0], v[1], v[2]);
}
