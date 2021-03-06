#include "quaternion.h"


extern void qat_add(vec4_t q1, vec4_t q2, vec4_t out)
{
	vec4_t c;
	c[0] = -q1[1] * q2[1] - q1[2] * q2[2] - q1[3] * q2[3] + q1[0] * q2[0];
	c[1] =  q1[1] * q2[0] + q1[2] * q2[3] - q1[3] * q2[2] + q1[0] * q2[1];
	c[2] = -q1[1] * q2[3] + q1[2] * q2[0] + q1[3] * q2[1] + q1[0] * q2[2];
	c[3] =  q1[1] * q2[2] - q1[2] * q2[1] + q1[3] * q2[0] + q1[0] * q2[3];
	vec4_cpy(out, c);
}


extern void qat_interp(vec4_t q1, vec4_t q2, float p, vec4_t out)
{
	vec4_t conv = {1.0, 0.0, 0.0, 0.0};
	float dot = vec4_dot(q1, q2);
	float pi = 1.0 - p;

	if(dot < 0) {
		conv[0] = pi * q1[0] + p * -q2[0];
		conv[1] = pi * q1[1] + p * -q2[1];
		conv[2] = pi * q1[2] + p * -q2[2];
		conv[3] = pi * q1[3] + p * -q2[3];
	} else {
		conv[0] = pi * q1[0] + p * q2[0];
		conv[1] = pi * q1[1] + p * q2[1];
		conv[2] = pi * q1[2] + p * q2[2];
		conv[3] = pi * q1[3] + p * q2[3];
	}

	vec4_nrm(conv, out);
}
