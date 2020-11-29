#include "collision.h"
#include "mbasic.h"
#include <stdio.h>
#include <stdlib.h>


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
	pln->equation[3] = -(nrm[0] * p0[0] + nrm[1] * p0[1] + nrm[2] * p0[2]);
	return 0;
}


extern int col_create_nrm(struct col_pln *pln, vec3_t p, vec3_t nrm)
{
	/* Copy normal vector */
	vec3_nrm(nrm, nrm);
	vec3_cpy(pln->normal, nrm);

	/* Set origin */
	vec3_cpy(pln->origin, p);

	/* Set equation */
	pln->equation[0] = nrm[0];
	pln->equation[1] = nrm[1];
	pln->equation[2] = nrm[2];
	pln->equation[3] = -(nrm[0] * p[0] + nrm[1] * p[1] + nrm[2] * p[2]);
	return 0;
}


extern int col_facing(struct col_pln *pln, vec3_t dir)
{
	float dot = vec3_dot(pln->normal, dir);
	return (dot <= 0);
}


extern float col_dist(struct col_pln *pln, vec3_t p)
{
	/*  */
	return vec3_dot(p, pln->normal) + pln->equation[3];
}



extern int col_init_pck(struct col_pck *pck, vec3_t pos, vec3_t vel, vec3_t e)
{
	vec3_cpy(pck->eRadius, e);

	vec3_cpy(pck->R3Position, pos);
	vec3_cpy(pck->R3Velocity, vel);

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


extern int pnt_in_trig(vec3_t pnt, vec3_t pa, vec3_t pb, vec3_t pc)
{
	vec3_t v0;
	vec3_t v1;
	vec3_t v2;

	float d00;
	float d01;
	float d02;
	float d11;
	float d12;

	float invDenom;
	float u;
	float v;

	vec3_sub(pc, pa, v0);
	vec3_sub(pb, pa, v1);
	vec3_sub(pnt, pa, v2);

	d00 = vec3_dot(v0, v0);
	d01 = vec3_dot(v0, v1);
	d02 = vec3_dot(v0, v2);
	d11 = vec3_dot(v1, v1);
	d12 = vec3_dot(v1, v2);

	invDenom = 1 / (d00 * d11 - d01 * d01);
	u = (d11 * d02 - d01 * d12) * invDenom;
	v = (d00 * d12 - d01 * d02) * invDenom;

	return (u >= 0) && (v >= 0) && (u + v < 1);
}


static int _min_root(float a, float b, float c, float maxR, float *root)
{
	/* Check if a solution exists */
	float determinant = b * b - 4.0f * a * c;
	float sqrtD;
	float r1;
	float r2;

	/* If determinant is negative it means no solutions. */ 
	if(determinant < 0.0f)
		return 0;

	/* 
	 * Calculate the two roots: (if determinant == 0 then
	 * x1==x2 but let’s disregard that slight optimization)
	 */
	sqrtD = POW2(determinant);

	r1 = (-b - sqrtD) / (2 * a);
	r2 = (-b + sqrtD) / (2 * a);

	/* Sort so x1 <= x2 */
	if(r1 > r2) {
		float tmp = r2;
		r2 = r1;
		r1 = tmp;
	}

	/* Get lowest root: */
	if(r1 > 0 && r1 < maxR) {
		*root = r1;
		return 1;
	}

	/* It is possible that we want x2 - this can happen if x1 < 0 */
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

	float distToPlane;
	float nrmDotVel;

	vec3_t tmp0;
	vec3_t tmp1;

	char embeddedInPlane = 0;
	vec3_t collisionPoint;
	char foundCollision = 0;

	float t = 1.0;

	float t0;
	float t1;

	/* Construct plane */
	col_create_pnt(&pln, p0, p1, p2);

	/* Calculate the signed distance */
	distToPlane = col_dist(&pln, pck->basePoint);

	/* Calculate denominator */
	nrmDotVel = vec3_dot(pln.normal, pck->velocity);

	/* If sphere is traveling parallel to the plane */
	if(nrmDotVel == 0.0) {
		if(ABS(distToPlane) >= 1.0) {
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
		t0 = (-1.0 - distToPlane) / nrmDotVel;
		t1 = ( 1.0 - distToPlane) / nrmDotVel;

		/* Swap so t0 < t1 */
		if(t0 > t1) {
			float swp = t1;
			t1 = t0;
			t0 = swp;
		}

		/* Check that atleast one result is in range */
		if(t0 > 1.0 || t1 < 0.0) {
			/* 
			 * Both t values are outside values [0,1]
			 * No collision possible!
			 */
			return;
		}

		/* Clamp to [0, 1] */
		t0 = clamp(t0);
		t1 = clamp(t1);
	}


	/*
	 * OK, at this point we have two time values t0 and t1 between which the
	 * swept sphere intersects with the triangle plane. If any collision is
	 * to occur it must// happen within this interval.
	 */
	t = 1.0;
	foundCollision = 0;

	/* 
	 * First we check for the easy case - collision inside the
	 * triangle. If this happens it must be at time t0 as this is
	 * when the sphere rests on the front side of the triangle
	 * plane. Note, this can only happen if the sphere is not
	 * embedded in the triangle plane.
	 */
	if(!embeddedInPlane) {
		vec3_t interPnt;

		/* Calculate intersection-point */
		vec3_sub(pck->basePoint, pln.normal, tmp0);
		vec3_scl(pck->velocity, t0, tmp1);
		vec3_add(tmp0, tmp1, interPnt);

		/* Intersection-point in triangle */
		if(pnt_in_trig(interPnt, p0, p1, p2)) {
			foundCollision = 1;
			t = t0;
			vec3_cpy(collisionPoint, interPnt);
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
		vec3_t posToVtx;
		float edgeSqr;
		float edgeDotVel;
		float edgeDotPosToVtx;
		float velDot;

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

		/* Point 1 */
		vec3_sub(base, p0, tmp0);
		b = 2.0 * vec3_dot(velocity, tmp0);
		vec3_sub(p0, base, tmp0);
		c = vec3_sqrlen(tmp0) - 1.0;

		if(_min_root(a, b, c, t, &newT)) {
			foundCollision = 1;
			t = newT;
			vec3_cpy(collisionPoint, p0);
			printf("With point 0\n");
		}

		/* Point 1 */
		vec3_sub(base, p1, tmp0);
		b = 2.0 * vec3_dot(velocity, tmp0);
		vec3_sub(p1, base, tmp0);
		c = vec3_sqrlen(tmp0) - 1.0;

		if(_min_root(a, b, c, t, &newT)) {
			foundCollision = 1;
			t = newT;
			vec3_cpy(collisionPoint, p1);
			printf("With point 1\n");
		}

		/* Point 2 */
		vec3_sub(base, p2, tmp0);
		b = 2.0 * vec3_dot(velocity, tmp0);
		vec3_sub(p2, base, tmp0);
		c = vec3_sqrlen(tmp0) - 1.0;

		if(_min_root(a, b, c, t, &newT)) {
			foundCollision = 1;
			t = newT;
			vec3_cpy(collisionPoint, p2);
			printf("With point 2\n");
		}

		/*
		 * Check edges
		 */

		velDot = vec3_dot(velocity, posToVtx);

		/* v0 --> v1 */
		vec3_sub(p1, p0, edge);
		vec3_sub(p0, base, posToVtx);
		edgeSqr = vec3_sqrlen(edge);
		edgeDotVel = vec3_dot(edge, velocity);
		edgeDotPosToVtx = vec3_dot(edge, posToVtx);

		a = edgeSqr * -velSqrLen + edgeDotVel * edgeDotVel;
		b = edgeSqr * (2.0 * velDot) - 2.0 * edgeDotVel * edgeDotPosToVtx;
		c = edgeSqr * (1.0 - vec3_sqrlen(posToVtx)) + edgeDotPosToVtx * edgeDotPosToVtx;

		if(_min_root(a, b, c, t, &newT)) {
			/* Check if intersection within line segment */
			float f = (edgeDotVel * newT - edgeDotPosToVtx) / edgeSqr;

			if(f >= 0.0 && f <= 1.0) {
				/* Intersection took place in segment */
				foundCollision = 1;
				t = newT;

				vec3_scl(edge, f, tmp0);
				vec3_add(p0, tmp0, collisionPoint);

				printf("With edge 0\n");
			}
		}

		/* v1 --> v2 */
		vec3_sub(p2, p1, edge);
		vec3_sub(p1, base, posToVtx);
		edgeSqr = vec3_sqrlen(edge);
		edgeDotVel = vec3_dot(edge, velocity);
		edgeDotPosToVtx = vec3_dot(edge, posToVtx);

		a = edgeSqr * -velSqrLen + edgeDotVel * edgeDotVel;
		b = edgeSqr * (2.0 * velDot) - 2.0 * edgeDotVel * edgeDotPosToVtx;
		c = edgeSqr * (1.0 - vec3_sqrlen(posToVtx)) + edgeDotPosToVtx * edgeDotPosToVtx;

		if(_min_root(a, b, c, t, &newT)) {
			/* Check if intersection within line segment */
			float f = (edgeDotVel * newT - edgeDotPosToVtx) / edgeSqr;

			if(f >= 0.0 && f <= 1.0) {
				/* Intersection took place in segment */
				foundCollision = 1;
				t = newT;

				vec3_scl(edge, f, tmp0);
				vec3_add(p1, tmp0, collisionPoint);

				printf("With edge 0\n");
			}
		}

		/* v2 --> v0 */
		vec3_sub(p0, p2, edge);
		vec3_sub(p2, base, posToVtx);
		edgeSqr = vec3_sqrlen(edge);
		edgeDotVel = vec3_dot(edge, velocity);
		edgeDotPosToVtx = vec3_dot(edge, posToVtx);

		a = edgeSqr * -velSqrLen + edgeDotVel * edgeDotVel;
		b = edgeSqr * (2.0 * velDot) - 2.0 * edgeDotVel * edgeDotPosToVtx;
		c = edgeSqr * (1.0 - vec3_sqrlen(posToVtx)) + edgeDotPosToVtx * edgeDotPosToVtx;

		if(_min_root(a, b, c, t, &newT)) {
			/* Check if intersection within line segment */
			float f = (edgeDotVel * newT - edgeDotPosToVtx) / edgeSqr;

			if(f >= 0.0 && f <= 1.0) {
				/* Intersection took place in segment */
				foundCollision = 1;
				t = newT;

				vec3_scl(edge, f, tmp0);
				vec3_add(p2, tmp0, collisionPoint);

				printf("With edge 0\n");
			}
		}
	}

	if(foundCollision) {
		vec3_t del;
		float dist;

		vec3_sub(collisionPoint, pck->basePoint, del);
		dist = vec3_len(del);

		if(pck->foundCollision == 0 || pck->nearestDistance > dist) {
			pck->foundCollision = 1;
			pck->nearestDistance = dist;
			vec3_cpy(pck->intersectionPoint, collisionPoint);
		}
	}
}


extern void check_trigs(struct col_pck *pck, short num, vec3_t *trig)
{
	int i;
	int j;
	vec3_t vtx[3];

	for(i = 0; i < num; i++) {
		for(j = 0; j < 3; j++) {
			vec3_cpy(vtx[j], trig[i * 3 + j]);
			vec3_div(vtx[j], pck->eRadius, vtx[j]);
		}

		trig_check(pck, vtx[0], vtx[1], vtx[2]);
	}
}
