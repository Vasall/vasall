#ifndef _QUATERNION_H
#define _QUATERNION_H

#include "vector.h"


extern void qat_add(vec4_t q1, vec4_t q2, vec4_t out);

extern void qat_interp(vec4_t q1, vec4_t q2, float p, vec4_t out);

#endif
