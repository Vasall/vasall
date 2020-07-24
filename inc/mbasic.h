#ifndef _MBASIC_H
#define _MBASIC_H

#define ABS(x) ((x < 0) ? (-x) : (x))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define ROUND(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

extern float clamp(float v);
extern float _abs(float v);
extern float dist(float x, float y, float xp, float yp);
extern float round(float v);

#endif
