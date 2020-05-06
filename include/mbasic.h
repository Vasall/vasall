#ifndef _MBASIC_H
#define _MBASIC_H

typedef float rect_t[2];

extern float clamp(float v);
extern float _abs(float v);
extern float dist(float x, float y, float xp, float yp);
extern float round(float v);

#endif
