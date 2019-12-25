#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "vec.h"

/* 
 * Create a new 3d-vector and fill the
 * created struct with the given values.
 *
 * @x: The x-value of the vector
 * @y: The y-value of the vector
 * @z: The z-value of the vector
 *
 * Returns: The created vector
*/
Vec3 vecCreate(float x, float y, float z)
{
	Vec3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return(v);
}

/* 
 * Set the values of a 3d-vector.
 *
 * @v: A pointer to the vector
 * @x: The new x-value
 * @y: The new y-value
 * @z: The new z-value
*/
void vecSet(Vec3 *v, float x, float y, float z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

/* 
 * Copy the values from the second vector 
 * into the first.
 *
 * @v1: The vector to copy into
 * @v2: The vector to copy from
*/
void vecCpy(Vec3 *v1, Vec3 *v2)
{
	memcpy(v1, v2, sizeof(Vec3));
}

/* 
 * Add two vectors and write the result to 
 * the first vector.
 *
 * @v1: The first vector
 * @v2: The second vector
*/
void vecAdd(Vec3 *v1, Vec3 v2)
{
	v1->x += v2.x;
	v1->y += v2.y;
	v1->z += v2.z;
}

/* 
 * Add two vectors and return the result as
 * a new vector.
 *
 * @v1: The first vector
 * @v2: the second vector
 *
 * Returns: A new vector containing
 * 	the sum of both vectors
*/
Vec3 vecAddRet(Vec3 v1, Vec3 v2)
{
	Vec3 res;
	res.x = v1.x + v2.x;
	res.y = v1.y + v2.y;
	res.z = v1.z + v2.z;
	return(res);
}

/* 
 * Subtract one vector from another and write 
 * to the first vector. 
 *
 * @v1: The first vector
 * @v2: The second vector
*/
void vecSub(Vec3 *v1, Vec3 v2)
{
	v1->x -= v2.x;
	v1->y -= v2.y;
	v1->z -= v2.z;
}

/* 
 * Subtract one vector from another and return 
 * the result as a new vector. 
 *
 * @v1: The first vector
 * @v2: The second vector
 *
 * Returns: A new vector containing
 * 	the difference between the vectors
*/
Vec3 vecSubRet(Vec3 v1, Vec3 v2)
{
	Vec3 res;
	res.x = v1.x - v2.x;
	res.y = v1.y - v2.y;
	res.z = v1.z - v2.z;
	return(res);
}

/* 
 * Scale a vector by a factor and write the 
 * result to the first vector.
 *
 * @v: A pointer to the vector to scale
 * @f: The scaling factor
*/
void vecScl(Vec3 *v, float f)
{
	v->x *= f;
	v->y *= f;
	v->z *= f;
}

/* 
 * Scale a vector by a factor and return the 
 * result as a new vector.
 *
 * @v: The vector to scale
 * @f: The scaling factor
 *
 * Returns: A new vector containing the
 * 	product of the vector and the factor
*/
Vec3 vecSclRet(Vec3 v, float f)
{
	Vec3 res;
	res.x = v.x * f;
	res.y = v.y * f;
	res.z = v.z * f;
	return(res);
}

/*
 * Divide the vector by the given
 * factor. This function is the
 * inverse to the scale-procedure.
 *
 * @v: The vector to scale
 * @f: The scaling factor
 */
void vecInvScl(Vec3 *v, float f)
{
	v->x /= f;
	v->y /= f;
	v->z /= f;
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
	double len = (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
	return((float)sqrt(len));
}

/* 
 * Normalize a vector and write to the vector.
 *
 * @v: The vector to normalize
*/
void vecNrm(Vec3 *v)
{
	float len = vecMag(*v);
	v->x /= len;
	v->y /= len;
	v->z /= len;
}

/* 
 * Normalize a vector and return the result the
 * result as a new vector.
 *
 * @v: The vector to normalize
 *
 * Returns: The normalized vector
*/
Vec3 vecNrmRet(Vec3 v)
{
	Vec3 res;
	float len = vecMag(v);
	res.x = v.x / len;
	res.y = v.y / len;
	res.z = v.z / len;
	return(res);
}

/*
 * Calculate the cross-product and
 * then return the result as a new
 * vector.
 *
 * @v1: The first vector
 * @v2: The second vector
 *
 * Returns: A new vector containing
 * 	the resulting values
 */
Vec3 vecCross(Vec3 v1, Vec3 v2)
{
	Vec3 v;

	v.x = (v1.y * v2.z) - (v1.z * v2.y);
	v.y = (v1.z * v2.x) - (v1.x * v2.z);
	v.z = (v1.x * v2.y) - (v1.y * v2.x);

	return(v);
}

/*
 * Create a new 4x4 matrix and fill the
 * memory with zeros.
 *
 * Returns: The created matrix or NULL
 * 	if an error occurred
 */
Mat4 mat4Zero(void)
{
	Mat4 mat;
	
	mat = calloc(16, sizeof(float));
	if(mat == NULL) {
		return(NULL);
	}

	mat[15] = 1.0;

	return(mat);
}

/*
 * Create an identity matrix.
 *
 * Returns: The created matrix or NULL
 * 	if an error occurred
 */
Mat4 mat4Idt(void)
{
	Mat4 mat = mat4Zero();
	if(mat == NULL) {
		return(NULL);
	}

	mat[0x00] = 1.0;
	mat[0x05] = 1.0;
	mat[0x0a] = 1.0;
	mat[0x0f] = 1.0;

	return(mat);
}

/* 
 * Multiply two 4x4 matrices and write the 
 * result to first one. The previous values
 * will be overwritten.
 *
 * @m1: Pointer to the first matrix
 * @m2: The second matrix
*/
void mat4Mult(Mat4 m1, Mat4 m2)
{
	int i, j, k;
	Mat4 res = mat4Zero();

	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			res[j * 4 + i] = 0;
	
			for(k = 0; k < 4; k++) {
				res[j * 4 + i] += m1[k * 4 + i] * 
					m2[j * 4 + k];
			}
		}
	}
	
	m1 = res;
}

/* 
 * Multiply two matrices and return the 
 * result as a new 4x4 matrix.
 *
 * @m1: The first matrix
 * @m2: The second matrix
 *
 * Returns: The result as a new
 * 	4x4 matrix
*/
Mat4 mat4MultRet(Mat4 m1, Mat4 m2)
{
	int i, j, k;
	Mat4 res = mat4Zero();

	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			res[j * 4 + i] = 0;
	
			for(k = 0; k < 4; k++) {
				res[j * 4 + i] += m1[k * 4 + i] * 
					m2[j * 4 + k];
			}
		}
	}

	return(res);
}

/* 
 * Combine two matrices into one.
 *
 * @m1: The first matrix
 * @m2: The second matrix
 * @dst: The matrix to write the result to
*/
void mat4Combine(Mat4 m1, Mat4 m2, Mat4 dst)
{
	int i, j, k, v1, v2;

	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			for(k = 0; k < 4; k++) {
				v1 = m1[k * 4 + i];
				v2 = m2[j * 4 + k];
				dst[j * 4 + i] += v1 * v2; 
			}
		}
	}
}


/*
 * Display a 4x4 matrix in the console.
 *
 * @m: The matrix to display
 */
void mat4Print(Mat4 m)
{
	int r, c;
	for(r = 0; r < 4; r++) {
		for(c = 0; c < 4; c++) {
			printf("%6.2f ", m[r + c * 4]);
		}
		printf("\n");
	}
}
