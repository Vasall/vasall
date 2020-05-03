#include "render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int rat_init(void)
{
	if(shd_init() < 0)
		goto err_cleanup;

	if(tex_init() < 0)
		goto err_cleanup;

	return 0;

err_cleanup:
	rat_close();
	return -1;
}

extern void rat_close(void)
{
	shd_close();
	tex_close();	
}
