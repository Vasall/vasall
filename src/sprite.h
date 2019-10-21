#ifndef _SPRITE_H_
#define _SPRITE_H_

/*
 * A simple sprite-class representing a
 * spritesheet, or a single image as a
 * spritesheet with only a single frame.
*/
typedef struct Sprite {
	int w;				/* Width of a single frame in pixel */
	int h;				/* Height of a single frame in pixel */

	int f;				/* Amount of frames */
	char **buf;			/* Buffer containing pixel-data */
} Sprite;

#endif
