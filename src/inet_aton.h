/*
 * inet_aton.h - Windows doesn't have one. :(
 */

#ifndef __INET_ATON_H__
#define __INET_ATON_H__

struct in_addr;
extern int inet_aton(const char *cp_arg, struct in_addr *addr);

#endif
