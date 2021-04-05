#ifndef _COLLISION_H
#define _COLLISION_H

#include "vector.h"


/*
 * 
 * COLLISION-PLANE
 *
 */

struct col_pln {
	float equation[4];
	vec3_t origin;
	vec3_t normal;
};


/*
 * Create a collision-plane from 3 points.
 *
 * @pln: A pointer to the collision-plane to be created
 * @p0: The first point
 * @p1: The second point
 * @p2: The third point
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int col_pln_fpnt(struct col_pln *pln, vec3_t p0, vec3_t p1, vec3_t p2);


/*
 * Create a collision-plane from an origin-point and a normal-vector.
 *
 * @pln: A pointer to the collision-plane to be created
 * @p: The origin-point
 * @nrm: The normal-vector
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int col_pln_fnrm(struct col_pln *pln, vec3_t p, vec3_t nrm);


/*
 * Check if a direction-vector is facing the plane.
 *
 * @pln: Pointer to the plane
 * @dir: A direction-vector
 *
 * Returns: 1 if the vector hits the plane-front and 0 if not
 */
extern int col_pln_facing(struct col_pln *pln, vec3_t dir);


/*
 * Calculate the closest distance between a point and a given plane.
 *
 * @pln: Pointer to the plane
 * @p: A point
 *
 * Returns: The closest distance between the point and the plane
 */
extern float col_pln_dist(struct col_pln *pln, vec3_t p);


/*
 * 
 * COLLISION-PACKAGES
 *
 */


/*
 * Sphere-Collision-Package
 */

struct col_pck_sphere {
	short objSlot;
		
	/* Ellipsoid radius */
	vec3_t eRadius;

	/* Info about the move being requested (in R3) */
	vec3_t R3Vel;
	vec3_t R3Pos;

	/* Info about the move being requested (in eSpace) */
	vec3_t velocity;
	vec3_t normalizedVelocity;
	vec3_t basePoint;

	/* Hit information */
	char foundCollision;
	float nearestDistance;
	vec3_t colPnt;
	struct col_pln pln;
	float t;
};


/*
 * Initialize a collision-package used to check collision between an ellipsoid
 * and multiple triangles.
 *
 * @pck: Pointer to the package to be initialized
 * @pos: The center-position of the ellipsoid
 * @vel: The velocity-vector of the ellipsoid
 * @e: The sclaing-factors of the ellipsoid
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int col_init_pck_sphere(struct col_pck_sphere *pck, vec3_t pos, vec3_t vel, vec3_t e);


/*
 * Ray-Collision-Package
 */

struct col_pck_ray {
	vec3_t pos;
	vec3_t dir;

	char found;
	float col_t;
	vec3_t col_pnt;
};

/*
 * Initialize a collision-package used when checking collision between a ray and
 * multiple bounding-boxes.
 *
 * @pck: Pointer to the package to initialize
 * @pos: The origin of the ray
 * @dir: The direction-vector of the ray
 *
 * Returns: 0 on success or -1 if an error occurred
 */
extern int col_init_pck_ray(struct col_pck_ray *pck, vec3_t pos, vec3_t dir);


/*
 * 
 * COLLISION-CHECKS
 *
 */

/*
 * Box-to-Box-Check
 * Check if two axis-aligned-bounding-boxes overlap.
 *
 * @min1: The lower corner of the first box
 * @max1: The higher corner of the first box
 * @min2: The lower corner of the second box
 * @max2: The higher corner of the second box
 *
 * Returns: 1 if the boxes overlap and 0 if not
 */
extern int col_b2b_check(vec3_t min1, vec3_t max1, vec3_t min2, vec3_t max2);


/*
 * Sphere-to-Triangle-Check
 * Check if the sphere, as set in the given collision-package, intersets with
 * the triangled defined by the 3 given points, which have to be converted into
 * eSpace already. This function doesn't directly return anything, as the
 * results will be written to the collision-package.
 *
 * @pck: The collision-package containing data about the sphere
 * @p0: The first corner of the triangle
 * @p1: The second corner of the triangle
 * @p2: The third corner of the triangle
 */
extern void col_s2t_check(struct col_pck_sphere *pck, vec3_t p0, vec3_t p1, vec3_t p2);


/*
 * Check if the sphere, as set in the given collision-package, intersects with
 * any of the given triangles. The coordinates of the traingle-corners have to
 * be converted to eSpace already. This function will check all triangles as
 * write the intersection with the smallest distance to the collision-package.
 * The <num> parameter specifies the number of triangles, but as each triangle
 * is made up of 3 corner-points, the <trig> list has to be 3 times the size of
 * <num>.
 *
 * @pck: The collision-package containing data about the sphere
 * @num: The number of triiangles
 * @trig: The corner-points for all triangles
 */
extern void col_trigs_check(struct col_pck_sphere *pck, short num, vec3_t *trig);


/*
 * Ray-to-Box-Check
 * Check for collision between an outgoing ray and a bounding-box and write the
 * results to the given collision-package.
 *
 * @pck: A collision-package containing the ray-data and to write the result to
 * @min: The lower corner of the bounding-box
 * @max: The higher corner of the bounding-box
 */
extern void col_r2b_check(struct col_pck_ray *pck, vec3_t min, vec3_t max);


/*
 * Ray-to-Triangle-Check
 * Check if a ray intersects with a triangle and write the results to the given
 * package.
 *
 * @pck: The package containing the ray-data and to write the result to
 * @p0: The first corner of the triangle
 * @p1: The second corner of the triangle
 * @p2: The third corner of the triangle
 */
extern void col_r2t_check(struct col_pck_ray *pck, vec3_t p0, vec3_t p1,
		vec3_t p2);

#endif
