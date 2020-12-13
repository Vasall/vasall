#ifndef _COLLISION_H
#define _COLLISION_H

#include "vector.h"

struct col_pln {
	float equation[4];
	vec3_t origin;
	vec3_t normal;
};

extern int col_create_pnt(struct col_pln *pln, vec3_t p0, vec3_t p1, vec3_t p2);
extern int col_create_nrm(struct col_pln *pln, vec3_t p, vec3_t nrm);

extern int col_facing(struct col_pln *pln, vec3_t dir);
extern float col_signedDistanceTo(struct col_pln *pln, vec3_t p);


struct col_pck {
	/* Ellipsoid radius */
	vec3_t eRadius;

	/* Info about the move being requested (in R3) */
	vec3_t R3Velocity;
	vec3_t R3Position;

	/* Info about the move being requested (in eSpace) */
	vec3_t velocity;
	vec3_t normalizedVelocity;
	vec3_t basePoint;

	/* Hit information */
	char foundCollision;
	float nearestDistance;
	vec3_t intersectionPoint;

};

extern int col_init_pck(struct col_pck *pck, vec3_t pos, vec3_t vel, vec3_t e);

/*
 * Utility-functions for collision
 */

extern int col_overlap(vec3_t min1, vec3_t max1, vec3_t min2, vec3_t max2);

extern void trig_check(struct col_pck *pck, vec3_t p0, vec3_t p1, vec3_t p2);

extern void check_trigs(struct col_pck *pck, short num, vec3_t *trig);


#endif
