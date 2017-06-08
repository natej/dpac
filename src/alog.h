/*
 * alog.h
 */

#ifndef ALOG_H
#define ALOG_H

/* log message levels */
#define LL_DBG  0
#define LL_NORM 1
#define LL_NONE 2

void
alog(int level, char *fmt, ...);

#endif	/* ALOG_H */
