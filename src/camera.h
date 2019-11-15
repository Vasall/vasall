#ifndef VASALL_CAMERA_H
#define VASALL_CAMERA_H

/*
 * A camera-struct used to respresent
 * the game-view. This is used to keep
 * track of the relative render-position
 * and the zoom/scaling.
*/
typedef struct vsCamera {
	/* 
	 * The position the camera currently
	 * focuses on. This position is
	 * going to be in the center of the
	 * screen.
	*/
	Vec2 pos;

	/* 
	 * Current zoomlevel as a multible
	 * of 1. The final render-size of
	 * the sprites is equal to:
	 * size in px = (zoom * tilesz) * <size in units>
	*/
	float zoom;

	/* 
	 * Width and height of a single tile 
	 * in pixels.
	*/
	int tilesz;
} vsCamera;

/* ==== DEFINE PROTOTYPES ==== */

/* 
 * Convert a relative position in units
 * to an absolute position on the screen
 * in pixel.
*/
extern void cam_getpos(vsCamera *cam, float x, float y, int *xp, int *yp);

/*
 * Convert a relative size in units
 * to an absolute size size at the
 * current zoom-level. 
*/
extern void cam_convun(vsCamera *cam, float x, float y, int *xp, int *yp);

#endif
