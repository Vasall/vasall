#ifndef _ERROR_H
#define _ERROR_H

#include "network.h"

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>

#define ERR_LOG(args)                                                          \
	do { (printf("[ERROR] %s:%d : ", __FILE__, __LINE__),                  \
			printf args, printf(" (%s)\n",                         \
				strcmp(SDL_GetError(), "") ? SDL_GetError() : \
				(lcp_errno > 0 ? lcp_strerr(lcp_errno) :       \
				 (errno > 0 ? strerror(errno) : "Success")))); \
	} while(0)

#endif
