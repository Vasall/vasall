#include "collision.h"
#include "mbasic.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>


extern int col_create_pnt(struct col_pln *pln, vec3_t p0, vec3_t p1, vec3_t p2)
{
	vec3_t tmp0;
	vec3_t tmp1;
	vec3_t nrm;

	/* Calculate normal */
	vec3_sub(p1, p0, tmp0);
	vec3_sub(p2, p0, tmp1);
	vec3_cross(tmp0, tmp1, nrm);
	vec3_nrm(nrm, nrm);

	vec3_cpy(pln->normal, nrm);

	/* Calculate origin */
	vec3_cpy(pln->origin, p0);

	/* Set equation */
	pln->equation[0] = nrm[0];
	pln->equation[1] = nrm[1];
	pln->equation[2] = nrm[2];
	pln->equation[3] = -vec3_dot(nrm, p0);
	return 0;
}


extern int col_create_nrm(struct col_pln *pln, vec3_t p, vec3_t nrm)
{
	vec3_t pnrm;

	/* Copy normal vector */
	vec3_nrm(nrm, pnrm);
	vec3_cpy(pln->normal, pnrm);

	/* Set origin */
	vec3_cpy(pln->origin, p);

	/* Set equation */
	pln->equation[0] = pnrm[0];
	pln->equation[1] = pnrm[1];
	pln->equation[2] = pnrm[2];
	pln->equation[3] = -vec3_dot(pnrm, p);
	return 0;
}


extern int col_facing(struct col_pln *pln, vec3_t dir)
{
	return (vec3_dot(pln->normal, dir) <= 0);
}


extern float col_dist(struct col_pln *pln, vec3_t p)
{
	return vec3_dot(p, pln->normal) + pln->equation[3];
}



extern int col_init_pck(struct col_pck *pck, vec3_t pos, vec3_t vel, vec3_t e)
{
	vec3_cpy(pck->eRadius, e);

	vec3_cpy(pck->R3Pos, pos);
	vec3_cpy(pck->R3Vel, vel);

	vec3_div(vel, e, pck->velocity);
	vec3_nrm(pck->velocity, pck->normalizedVelocity);
	vec3_div(pos, e, pck->basePoint);

	pck->foundCollision = 0;
	return 0;
}


extern int col_overlap(vec3_t min1, vec3_t max1, vec3_t min2, vec3_t max2)
{
	return (min1[0] <= max2[0] && max1[0] >= min2[0]) &&
		(min1[1] <= max2[1] && max1[1] >= min2[1]) &&
		(min1[2] <= max2[2] && max1[2] >= min2[2]);
}


static int pnt_in_trig(vec3_t point, vec3_t p1, vec3_t p2, vec3_t p3)
{
	vec3_t u, v, w, vw, vu, uw, uv;
	float d, r, t;

	vec3_sub(p2, p1, u);
	vec3_sub(p3, p1, v);
	vec3_sub(point, p1, w);


	vec3_cross(v, w, vw);
	vec3_cross(v, u, vu);

	if(vec3_dot(vw, vu) < 0.0f)
		return 0;

	vec3_cross(u, w, uw);
	vec3_cross(u, v, uv);

	if(vec3_dot(uw, uv) < 0.0f)
		return 0;

	d = vec3_len(uv);
	r = vec3_len(vw) / d;
	t = vec3_len(uw) / d;

	return ((r + t) <= 1);
}

static int _min_root(float a, float b, float c, float maxR, float *root)
{
	/* Check if a solution exists */
	float determinant = b * b - 4.0f * a * c;
	float sqrtD, r1, r2;

	/* If determinant is negative it means no solutions */
	if(determinant < 0.0f)
		return 0;

	/* 
	 * calculate the two roots: (if determinant == 0 then
	 * x1==x2 but let’s disregard that slight optimization)
	 */
	sqrtD = sqrt(determinant);
	r1 = (-b - sqrtD) / (2 * a);
	r2 = (-b + sqrtD) / (2 * a);

	/* Sort so x1 <= x2 */
	if(r1 > r2) {
		float temp = r2;
		r2 = r1;
		r1 = temp;
	}
	/* Get lowest root: */
	if(r1 > 0 && r1 < maxR) {
		*root = r1;
		return 1;
	}
	/* It is possible that we want x2 - this can happen  if x1 < 0 */
	if(r2 > 0 && r2 < maxR) {
		*root = r2;
		return 1;
	}
	/* No (valid) solutions */
	return 0;
}

/* Check if a collision between the sphere and triangle occurrs */
extern void trig_check(struct col_pck *pck, vec3_t p0, vec3_t p1, vec3_t p2)
{
	struct col_pln pln;

	vec3_t tmp0;
	vec3_t tmp1;

	/* Construct plane */
	col_create_pnt(&pln, p0, p1, p2);

	/* Is triangle front-facing to the velocity vector? */
	if(col_facing(&pln, pck->normalizedVelocity)) {
		double t0, t1;
		char embeddedInPlane = 0;

		/* Calculate the signed distance */
		double signedDistToTrianglePlane = col_dist(&pln, pck->basePoint);

		/* Calculate denominator */
		float normalDotVelocity = vec3_dot(pln.normal, pck->velocity);

		float t = 1.0;
		int foundCollision = 0;
		vec3_t collisionPoint;

		/* If sphere is traveling parallel to the plane */
		if(normalDotVelocity == 0.0f) {
			if(fabs(signedDistToTrianglePlane) >= 1.0f) {
				/* 
				 * Sphere is not embedded in plane.
				 * No collision possible!
				 */
				return;
			}
			else {
				/*
				 * Sphere is embedded in plane.
				 * It intresects in the whole range [0..1]
				 */
				embeddedInPlane = 1;
				t0 = 0.0;
				t1 = 1.0;
			}
		}
		else {
			/* N dot D is not 0. Calculate intersection interval: */
			t0 = (-1.0 - signedDistToTrianglePlane) / normalDotVelocity;
			t1 = ( 1.0 - signedDistToTrianglePlane) / normalDotVelocity;

			/* Swap so t0 < t1 */
			if(t0 > t1) {
				double swp = t1;
				t1 = t0;
				t0 = swp;
			}

			/* Check that atleast one result is in range */
			if(t0 > 1.0f || t1 < 0.0f) {
				/* 
				 * Both t values are outside values [0,1]
				 * No collision possible!
				 */
				return;
			}

			/* Clamp to [0, 1] */
			if (t0 < 0.0) t0 = 0.0;
			if (t1 < 0.0) t1 = 0.0;
			if (t0 > 1.0) t0 = 1.0;
			if (t1 > 1.0) t1 = 1.0;
		}

		/* 
		 * First we check for the easy case - collision inside the
		 * triangle. If this happens it must be at time t0 as this is
		 * when the sphere rests on the front side of the triangle
		 * plane. Note, this can only happen if the sphere is not
		 * embedded in the triangle plane.
		 */
		if(!embeddedInPlane) {
			vec3_t PlaneIntersectionPoint;

			/* Calculate intersection-point */
			vec3_sub(pck->basePoint, pln.normal, tmp0);
			vec3_scl(pck->velocity, t0, tmp1);
			vec3_add(tmp0, tmp1, PlaneIntersectionPoint);

			/* Intersection-point in triangle */
			if(pnt_in_trig(PlaneIntersectionPoint, p0, p1, p2)) {
				foundCollision = 1;
				t = t0;
				vec3_cpy(collisionPoint, PlaneIntersectionPoint);
			}
		}

		/*
		 * If we haven’t found a collision already we’ll have to sweep
		 * sphere against points and edges  of the triangle. Note: A
		 * collision inside the triangle (the check above) will always
		 * happen before a vertex or edge collision! This is why we can
		 * skip the swept test if the above// gives a collision!
		 */
		if(foundCollision == 0) {
			float a;
			float b;
			float c;
			float newT;
			vec3_t velocity;
			vec3_t base;
			float velSqrLen;

			vec3_t edge;
			vec3_t baseToVtx;
			float edgeSqrLen;
			float edgeDotVel;
			float edgeDotBaseToVtx;

			/*
			 * Set variables for checking points.
			 */
			vec3_cpy(velocity, pck->velocity);
			vec3_cpy(base, pck->basePoint);
			velSqrLen = vec3_sqrlen(velocity);

			/*
			 * For each vertex or edge a quadratic equation have to
			 * be solved. We parameterize this equation as 
			 * a * t^2 + b * t + c = 0 and below we calculate the 
			 * parameters a,b and c for each test.
			 */

			/*
			 * Check Points
			 */

			a = velSqrLen;

			/* Point 0 */
			vec3_sub(base, p0, tmp0);
			b = 2.0 * vec3_dot(velocity, tmp0);
			vec3_sub(p0, base, tmp0);
			c = vec3_sqrlen(tmp0) - 1.0;

			if(_min_root(a, b, c, t, &newT)) {
				t = newT;
				foundCollision = 1;
				vec3_cpy(collisionPoint, p0);
			}

			/* Point 1 */
			vec3_sub(base, p1, tmp0);
			b = 2.0 * vec3_dot(velocity, tmp0);
			vec3_sub(p1, base, tmp0);
			c = vec3_sqrlen(tmp0) - 1.0;

			if(_min_root(a, b, c, t, &newT)) {
				t = newT;
				foundCollision = 1;
				vec3_cpy(collisionPoint, p1);
			}

			/* Point 2 */
			vec3_sub(base, p2, tmp0);
			b = 2.0 * vec3_dot(velocity, tmp0);
			vec3_sub(p2, base, tmp0);
			c = vec3_sqrlen(tmp0) - 1.0;

			if(_min_root(a, b, c, t, &newT)) {
				t = newT;
				foundCollision = 1;
				vec3_cpy(collisionPoint, p2);
			}

			/*
			 * Check edges
			 */

			/* p0 --> p1 */
			vec3_sub(p1, p0, edge);
			vec3_sub(p0, base, baseToVtx);
			edgeSqrLen = vec3_sqrlen(edge);
			edgeDotVel = vec3_dot(edge, velocity);
			edgeDotBaseToVtx = vec3_dot(edge, baseToVtx);

			a = edgeSqrLen * -velSqrLen + edgeDotVel * edgeDotVel;
			b = edgeSqrLen * (2 * vec3_dot(velocity, baseToVtx)) - 2.0 * edgeDotVel * edgeDotBaseToVtx;
			c = edgeSqrLen * (1 - vec3_sqrlen(baseToVtx)) + edgeDotBaseToVtx * edgeDotBaseToVtx;

			if(_min_root(a, b, c, t, &newT)) {
				/* Check if intersection within line segment */
				float f = (edgeDotVel * newT - edgeDotBaseToVtx) / edgeSqrLen;

				if(f >= 0.0 && f <= 1.0) {
					/* Intersection took place in segment */
					t = newT;
					foundCollision = 1;

					vec3_scl(edge, f, tmp0);
					vec3_add(p0, tmp0, collisionPoint);
				}
			}

			/* p1 --> p2 */
			vec3_sub(p2, p1, edge);
			vec3_sub(p1, base, baseToVtx);
			edgeSqrLen = vec3_sqrlen(edge);
			edgeDotVel = vec3_dot(edge, velocity);
			edgeDotBaseToVtx = vec3_dot(edge, baseToVtx);

			a = edgeSqrLen * -velSqrLen + edgeDotVel * edgeDotVel;
			b = edgeSqrLen * (2 * vec3_dot(velocity, baseToVtx)) - 2.0 * edgeDotVel * edgeDotBaseToVtx;
			c = edgeSqrLen * (1 - vec3_sqrlen(baseToVtx)) + edgeDotBaseToVtx * edgeDotBaseToVtx;

			if(_min_root(a, b, c, t, &newT)) {
				/* Check if intersection within line segment */
				float f = (edgeDotVel * newT - edgeDotBaseToVtx) / edgeSqrLen;

				if(f >= 0.0 && f <= 1.0) {
					/* Intersection took place in segment */
					t = newT;
					foundCollision = 1;

					vec3_scl(edge, f, tmp0);
					vec3_add(p1, tmp0, collisionPoint);
				}
			}

			/* p2 --> p0 */
			vec3_sub(p0, p2, edge);
			vec3_sub(p2, base, baseToVtx);
			edgeSqrLen = vec3_sqrlen(edge);
			edgeDotVel = vec3_dot(edge, velocity);
			edgeDotBaseToVtx = vec3_dot(edge, baseToVtx);

			a = edgeSqrLen * -velSqrLen + edgeDotVel * edgeDotVel;
			b = edgeSqrLen * (2 * vec3_dot(velocity, baseToVtx)) - 2.0 * edgeDotVel * edgeDotBaseToVtx;
			c = edgeSqrLen * (1 - vec3_sqrlen(baseToVtx)) + edgeDotBaseToVtx * edgeDotBaseToVtx;

			if(_min_root(a, b, c, t, &newT)) {
				/* Check if intersection within line segment */
				float f = (edgeDotVel * newT - edgeDotBaseToVtx) / edgeSqrLen;

				if(f >= 0.0 && f <= 1.0) {
					/* Intersection took place in segment */
					t = newT;
					foundCollision = 1;

					vec3_scl(edge, f, tmp0);
					vec3_add(p2, tmp0, collisionPoint);
				}
			}
		}

		if(foundCollision == 1) {
			float dist;

			dist = t * vec3_len(pck->velocity);

			if(pck->foundCollision == 0 || dist < pck->nearestDistance) {
				pck->foundCollision = 1;
				pck->nearestDistance = dist;
				pck->t = t;
				vec3_cpy(pck->colPnt, collisionPoint);
				pck->pln = pln;
			}
		}
	}
}
