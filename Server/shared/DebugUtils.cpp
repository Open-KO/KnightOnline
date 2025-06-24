#include "stdafx.h"
#include <stdarg.h>

void FormattedDebugString(const char* fmt, ...)
{
	char buf[4096], *p = buf;
	va_list args;
	int n;

	va_start(args, fmt);
	n = _vsnprintf(p, sizeof(buf) - 3, fmt, args); // allow for proper linefeed & null terminator
	va_end(args);
	p += (n < 0) ? sizeof(buf) - 3 : n;
	*p++ = '\r';
	*p++ = '\n';
	*p   = '\0';

#ifdef WIN32
	OutputDebugStringA(buf);
#else
	printf("%s", buf);
#endif
}
