#include <wimp_log.h>
#include <stdarg.h>
#include <stdio.h>

void wimp_log(const char* format, ...)
{
	va_list arg;
	int done;

	va_start (arg, format);
	done = vfwimp_log(stdout, format, arg);
	va_end (arg);
}