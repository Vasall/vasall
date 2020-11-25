#include "collision.h"
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

	vec3_cpy(pln->normal, nrm);
	vec3_nrm(nrm, pln->normal);

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
