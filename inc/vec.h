#ifndef _VEC_H
#define _VEC_H

#include "mat.h"

#define M_PI 3.141592654
#define TO_RADS (3.14/180.0)

#define VEC2_SIZE (sizeof(float)*2)
typedef float vec2_t[2];

#define VEC3_SIZE (sizeof(float)*3)
typedef float vec3_t[3];

#define INT2 (sizeof(int)*2)
typedef int int2_t[2];

#define INT3 (sizeof(int)*3)
typedef int int3_t[3];

/*
 * Set the values of a 3d-vector.
 *
 * @v: The vector to the the values of
 * @x: The x-value of the vector
 * @y: The y-value of the vector
 * @z: The z-value of the vector
 */
extern void vec3_set(vec3_t v, float x, float y, float z);

/*
 * Copy the values of one 3d-vector into another.
 *
 * @dst: The vector to copy the values to
 * @src: The vector to copy the values from
 */
extern void vec3_cpy(vec3_t dst, vec3_t src);

/*
 * Add up two 3d-vectors and return the resulting vector. The resulting vector
 * can be the same as one of the input-vectors.
 *
 * @v1: The first vector
 * @v2: The second vector
 * @res: A vector to write the result to
 */
extern void vec3_add(vec3_t v1, vec3_t v2, vec3_t res);

/*
 * Subtract one 3d-vector from another and return the resulting vector.
 *
 * @v1: The first vector
 * @v2: The second vector
 * @res: A vector to write the result to
 */
extern void vec3_sub(vec3_t v1, vec3_t v2, vec3_t res);

/*
 * Scale a 3d-vector by a factor and return the resulting vector.
 *
 * @v: The vector to scale
 * @f: The scaling factor
 * @res: A vector to write the result to
 */
extern void vec3_scl(vec3_t v, float f, vec3_t res);

/*
 * Divide a 3d-vector by a factor to make it smaller, then return the 
 * resulting vector. This is the inverse function to vec3_scl().
 *
 * @v: The vector to make smaller
 * @f: The scaling factor
 * @res: A vector to write the result to
 */
extern void vec3_inv_scl(vec3_t v, float f, vec3_t res);

/*
 * Get the magnitude of a 3d-vector.
 *
 * @v: The vector to get the magnitude of
 *
 * Returns: The magnitude of the vector
 */
extern float vec3_mag(vec3_t v);

/*
 * Normalize a 3d-vector, so it's length is equal to 1.
 *
 * @v: The vector to normalize
 */
extern void vec3_nrm(vec3_t v, vec3_t res);

/*
 * Calculate the dot-product of two 3d-vectors.
 *
 * @v1: The first vector
 * @v2: The second vector
 *
 * Returns: The dot-product of the two vectors
 */
extern float vec3_dot(vec3_t v1, vec3_t v2);

/*
 * Calculate the cross-product of two 3d-vectors and return the resulting
 * vector. The result-vector can be the same as one of the input-vectors.
 *
 * @v1: The first vector
 * @v2: The second vector
 * @res: A vector to write the result to
 */
extern void vec3_cross(vec3_t v1, vec3_t v2, vec3_t res);

/*
 * Rotate a 3d-vector relative to the x-axis, then return the resulting
 * vector. The result-vector can be the same as the input-vector.
 * 
 * @v: The vector to rotate
 * @angle: The angle in radians
 * @res: A vector to write the result to
 */
extern void vec3_rot_x(vec3_t v, float angle, vec3_t res);

/*
 * Rotate a 3d-vector relative to the y-axis, then return the resulting
 * vector. The result-vector can be the same as the input-vector.
 * 
 * @v: The vector to rotate
 * @angle: The angle in radians
 * @res: A vector to write the result to
 */
extern void vec3_rot_y(vec3_t v, float angle, vec3_t res);

/*
 * Rotate a 3d-vector relative to the z-axis, then return the resulting
 * vector. The result-vector can be the same as the input-vector.
 * 
 * @v: The vector to rotate
 * @angle: The angle in radians
 * @res: A vector to write the result to
 */
extern void vec3_rot_z(vec3_t v, float angle, vec3_t res);

/*
 * Rotate a 3d-vector relative to all 3 axes, then return the resulting 
 * vector. The result-vector can be the same as the input-vector.
 *
 * @v: The vector to rotate
 * @angle: The angle to rotate the vector with
 * @axis: The axis to rotate the vector on
 * @res: A vector to write the result to
 */
extern void vec3_rot_axes(vec3_t v, float angle, vec3_t axis, vec3_t res);

/*
 * Transform a 3d-vector with a 3x3-matrix and return the resulting vector.
 * The result vector can be the same as the input-vector.
 *
 * @v: The vector to transform
 * @mat: The transforming matrix
 * @res: A vector to write the result vector to
 */
extern void vec3_trans(vec3_t v, mat3_t mat, vec3_t res);

/*
 * Display a vector in the terminal.
 *
 * @v: The vector to display
 */
extern void vec3_print(vec3_t v);

/*
 * Get the height at a position in a triangle formed by the corners
 * p1, p2 and p3.
 *
 * @p1: A vector representing the first corner
 * @p2: A vector representing the second corner
 * @p3: A vector representing the third corner
 * @pos: The position in the formed triangle
 *
 * Returns: The height at the position in the triangle
 */
extern float vec3_barry_centric(vec3_t p1, vec3_t p2, vec3_t p3, vec2_t pos);

#endif
