/*
 * app.h - Global application setup.
 */

#ifndef APP_H
#define APP_H

/*
 * Windows specific
 */
#ifdef WIN32

/* silence security warnings for use of certain functions */
#define _CRT_SECURE_NO_WARNINGS

/* map function names to Windows C runtime equivalents */
#define snprintf _snprintf
#define strdup _strdup
#define fileno _fileno

/* path separator */
#define PATH_SEP '\\'

/*
 * Non-Windows specific
 */
#else

/* path separator */
#define PATH_SEP '/'

#endif	/* WIN32 */

#endif	/* APP_H */
