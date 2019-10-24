#include <math.h>
#include <SDL.h>
#include "vector.h"

/*
 * Check, whether a specified position is inside of a rectangle. If the
 * point is just on the border, it will still be considered inside of the
 * rectangle.
 * 
 * @rect: The rectangle
 * @pos: The position
 *
 * Returns: 1 if position is inside of rectangle, 0 if it is outside
 */
char in_rect(SDL_Rect *rect, Vec2 *pos) 
{
	if(pos->x >= rect->x && pos->x <= rect->x + rect->w && 
			pos->y >= rect->y && pos->y <= rect->y + rect->h) {
		return (1);
	}
	return (0);
}

/*
 * Adds two vectors together.
 *
 * @dst: The destination vector
 * @vec: The vector to add to dst
 */
void vec_add(Vec2 *dst, Vec2 *vec) 
{
	 dst->x += vec->x;
	 dst->y += vec->y;
}

/*
 * Adds two vectors together and returns the result.
 *
 * @vec1: The first vector
 * @vec2: The second vector
 *
 * Returns: A new Vector, equal to vec1 + vec2
 */
Vec2 vec_add_ret(Vec2 *vec1, Vec2 *vec2)
{
	Vec2 retVal = {vec1->x + vec2->x, vec1->y + vec2->y};
	return (retVal);
}

/*
 * Subtracts one vector from another.
 *
 * @dst: The destination vector
 * @vec: The vector to subtract from dst
 */
void vec_sub(Vec2 *dst, Vec2 *vec) 
{
	 dst->x -= vec->x;
	 dst->y -= vec->y;
}

/*
 * Subtracts one vector from another and returns the result.
 *
 * @vec1: The first vector
 * @vec2: The second vector
 *
 * Returns: A new Vector, equal to vec1 - vec2
 */
Vec2 vec_sub_ret(Vec2 *vec1, Vec2 *vec2)
{
	Vec2 retVal = {vec1->x - vec2->x, vec1->y - vec2->y};
	return (retVal);
}

/*
 * Scales a vector by a factor.
 *
 * @dst: The destination vector
 * @fac: The factor to scale dst by
 */
void vec_scl(Vec2 *dst, float fac) 
{
	 dst->x *= fac;
	 dst->y *= fac;
}

/*
 * 
 * Scales a vector by a factor and returns the result.
 *
 * @vec: The vector to scale
 * @fac: The factor to scale the vector by
 *
 * Returns: A new vector, that is a scaled version of the input vector
 */
Vec2 vec_scl_ret(Vec2 *vec, float fac)
{
	Vec2 retVal = {vec->x * fac, vec->y * fac};
	return (retVal);
}

/*
 * Sets the x and y value of the destination vector to
 * the value of the respecçtive argument
 *
 * @dst: The destination vector to write into
 * @x: the new x value of the destination vector
 * @y: the new y value of the destination vector
 */
void vec_set(Vec2 *dst, float x, float y)
{
	dst->x = x;
	dst->y = y;
}

/*
 * Copies the values of the source vector into the destination vector.
 *
 * @dst: The destination vector
 * @src: The source vector
 */
void vec_cpy(Vec2 *dst, Vec2 *src)
{
	dst->x = src->x;
	dst->y = src->y;
}

/*
 * Determines the magnitude or length of a given vector.
 *
 * @vec: The input vector
 *
 * Returns: The magnitude or length of the input vector
 */
float vec_mag(Vec2 *vec)
{
	return sqrt(vec->x * vec->x + vec->y * vec->y);
}

/*
 * Computes the square of the magnitude of a given vector.
 *
 * @vec: The input vector
 *
 * Returns: The square of the magnitude of the input vector
 */
float vec_magsq(Vec2 *vec)
{
	return (vec->x * vec->x + vec->y * vec->y);
}

/*
 * Scales a vector, so a specific maximum magnitude is not
 * exceeded. So if the input vectors megnitude is smaller
 * than the limit, it will stay untouched.
 *
 * @vec: The input vector
 * @lim: The limit magnitude the input vector shall not exceed
 */
void vec_lim(Vec2 *vec, float lim)
{
	float mag = vec_mag(vec);
	if(mag > lim) {
		float fac = lim / mag;
		vec_scl(vec, fac);
	}
}

/*
 * Scales a vector, so a specific maximum magnitude is not
 * exceeded. So if the input vectors megnitude is smaller
 * than the limit, it will stay untouched.
 * This function then returns a new vector, that is limited
 * to lim in magnitude.
 *
 * @vec: The input vector
 * @lim: The limit magnitude the input vector shall not exceed
 *
 * Returns: A new vector, that is limited to a magnitude of lim
 */
Vec2 vec_lim_ret(Vec2 *vec, float lim)
{
	float mag = vec_mag(vec);
	if(mag > lim) {
		float fac = lim / mag;
		Vec2 limited = vec_scl_ret(vec, fac);
		return limited;
	} else {
		return *vec;
	}
}

/*
 * Computes the dot product of two given vectors
 *
 * @vec1: The first vector
 * @vec2: The second vector
 *
 * Returns: The dot product of the input vectors
 */
float vec_dot(Vec2 *vec1, Vec2 *vec2)
{
	return (vec1->x * vec2->x + vec1->y * vec2->y);
}

/*
 * Normalizes a vector, so it's magnitude is 1
 *
 * @vec: the vector to normalize
 */
void vec_nrm(Vec2 *vec)
{
	float mag = vec_mag(vec);
	if(mag != 1.0) {
		float fac = 1.0 / mag;
		vec_scl(vec, fac);
	}
}

/*
 * Normalizes a vector and returns it.
 * This function returns a new vector, that has the same angle
 * as the input vector, but a magnitude of 1.
 *
 * @vec: The input vector
 *
 * Returns: A new vector, that is the input vector scaled
 * 			to a magnitude of 1
 */
Vec2 vec_nrm_ret(Vec2 *vec)
{
	float mag = vec_mag(vec);
	if(mag != 1.0) {
		float fac = 1.0 / mag;
		Vec2 normed = vec_scl_ret(vec, fac);
		return normed;
	} else {
		return *vec;
	}
}

/*
 * Not yet implemented!
 *
 */
void vec_rot(Vec2 *vec, int deg)
{
	/* TODO */
}

/*
 * Not yet implemented!
 *
 */
Vec2 vec_rot_ret(Vec2 *vec, int deg)
{
	/* TODO */
}
