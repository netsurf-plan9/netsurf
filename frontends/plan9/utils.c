#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include "plan9/utils.h"

static int debug = 0;

void
DBG(const char *format, ...)
{
	char buf[1024];
	va_list ap;

	if(!debug)
		return;
	va_start(ap, format);
	vsnprintf(buf, sizeof buf - 1, format, ap);
	va_end(ap);
	fprintf(stderr, "(debug) %s\n", buf);
}
