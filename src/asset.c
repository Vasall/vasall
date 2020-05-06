#include "asset.h"

#include <stdlib.h>


extern int ast_init(void)
{
	if(tex_init() < 0)
		return -1;

	if(txt_init() < 0)
		goto err_close_tex;

	return 0;

err_close_tex:
	tex_close();
	return -1;
}

extern void ast_close(void)
{
	txt_close();
	tex_close();
}

