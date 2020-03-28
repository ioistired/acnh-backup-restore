#include "util.hpp"

#include <switch.h>
#include <stdio.h>
#include <stdarg.h>

int print(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int ret = vprintf(fmt, ap);
	va_end(ap);
	consoleUpdate(NULL);
	return ret;
}
