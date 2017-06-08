/*
 * alog.c - Application logging.
 */

#include "app.h"

#include <stdio.h>
#include <stdarg.h>

#include "dpac.h"
#include "alog.h"

extern char *g_app_basename;

/*
 * log message
 */
void
alog(int level, char *fmt, ...)
{

	va_list ap;

	if (level > APP_LOG_LEVEL)
		return;

	fprintf(stderr, "%s: ", g_app_basename);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");

return;
}
