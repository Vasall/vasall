#ifndef _ERROR_H
#define _ERROR_H

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>

#define ERR_LOG(args) do { (printf("[ERROR] %s:%d :: ", __FILE__, __LINE__), \
		printf args, printf(" (%s)\n", strerror(errno))); } while(0)

#endif
