#ifndef _MBASIC_H
#define _MBASIC_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ABS(x) ((x < 0) ? (-x) : (x))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define ROUND(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define SIGN(x) ((x/ABS(x)))
#define POW2(x) (x * x)

extern float clamp(float v);
extern float dist(float x, float y, float xp, float yp);
extern float round(float v);

#endif
