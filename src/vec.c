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

/*
 * Rotates the vector around the x axis by angle radians
 *
 * @v: The input vector
 * @angle: The angle to rotate by
 * @res: The resulting vector
 */
void vecRotX(Vec3 v, float angle, Vec3 res)
{
	Mat3 rmat;
	mat3Idt(rmat);
	rmat[0x4] = cos(angle);
	rmat[0x5] = -sin(angle);
	rmat[0x7] = sin(angle);
	rmat[0x8] = cos(angle);

	vecTransf(v, rmat, res);
}

/*
 * Rotates the vector around the y axis by angle radians
 *
 * @v: The input vector
 * @angle: The angle to rotate by
 * @res: The resulting vector
 */
void vecRotY(Vec3 v, float angle, Vec3 res)
{
	Mat3 rmat;
	mat3Idt(rmat);
	rmat[0x0] = cos(angle);
	rmat[0x2] = sin(angle);
	rmat[0x6] = -sin(angle);
	rmat[0x8] = cos(angle);

	vecTransf(v, rmat, res);
}

/*
 * Rotates the vector around the z axis by angle radians
 *
 * @v: The input vector
 * @angle: The angle to rotate by
 * @res: The resulting vector
 */
void vecRotZ(Vec3 v, float angle, Vec3 res)
{
	Mat3 rmat;
	mat3Idt(rmat);
	rmat[0x0] = cos(angle);
	rmat[0x1] = -sin(angle);
	rmat[0x3] = sin(angle);
	rmat[0x4] = cos(angle);
	vecTransf(v, rmat, res);
}

/*
 * Rotates a vector around a specified axis
 *
 * @v: The input vector
 * @angle: The angle to rotate by
 * @axis: The rotation axis
 * @res: The destination vector to write to
 */
void vecRotAxis(Vec3 v, float angle, Vec3 axis, Vec3 res)
{
	Mat3 rmat;
	float q0, q1, q2, q3;

	vecNrm(axis, axis);
	q0 = cos(angle / 2);
	q1 = sin(angle / 2) * axis[0];
	q2 = sin(angle / 2) * axis[1];
	q3 = sin(angle / 2) * axis[2];

	rmat[0x0] = q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3;
	rmat[0x1] = 2 * (q1 * q2 - q0 * q3);
	rmat[0x2] = 2 * (q1 * q3 + q0 * q2);
	rmat[0x3] = 2 * (q2 * q1 + q0 * q3);
	rmat[0x4] = q0 * q0 - q1 * q1 + q2 * q2 - q3 * q3;
	rmat[0x5] = 2 * (q2 * q3 - q0 * q1);
	rmat[0x6] = 2 * (q3 * q1 - q0 * q2);
	rmat[0x7] = 2 * (q3 * q2 + q0 * q1);
	rmat[0x8] = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;
	
/*
	c = cos(angle);
	s = sin(angle);

	rmat[0x0] = c + axis[0] * axis[0] * (1 - c);
	rmat[0x1] = axis[0] * axis[1] * (1 - c) - axis[2] * s;
	rmat[0x2] = axis[0] * axis[2] * (1 - c) + axis[1] * s;
	rmat[0x3] = axis[0] * axis[1] * (1 - c) + axis[2] * s;
	rmat[0x4] = c + axis[1] * axis[1] * (1 - c);
	rmat[0x5] = axis[1] * axis[2] * (1 - c) + axis[0] * s;
	rmat[0x6] = axis[0] * axis[2] * (1 - c) - axis[1] * s;
	rmat[0x7] = axis[1] * axis[2] * (1 - c) + axis[0] * s;
	rmat[0x8] = c + axis[2] * axis[2] * (1 - c);
*/
	vecTransf(v, rmat, res);

}

void vecTransf(Vec3 v, Mat3 mat, Vec3 res)
{
	Vec3 tmp;
	vecCpy(tmp, v);

	res[0] = tmp[0] * mat[0x0] + tmp[1] * mat[0x1] + tmp[2] * mat[0x2];
	res[1] = tmp[0] * mat[0x3] + tmp[1] * mat[0x4] + tmp[2] * mat[0x5];
	res[2] = tmp[0] * mat[0x6] + tmp[1] * mat[0x7] + tmp[2] * mat[0x8];
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
