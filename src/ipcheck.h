/*
 * Renamed/edited for needed IP networks/CIDR functions.
 * Nathan Jennings, 2006-08-01
 *
 *
 * This file is part of GNUnet.
 * (C) 2001, 2002, 2003, 2004, 2005, 2006 Christian Grothoff (and other contributing authors)
 *
 * GNUnet is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNUnet is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNUnet; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * @file include/gnunet_util.h
 * @brief public interface to libgnunetutil
 *
 * @author Christian Grothoff
 * @author Krista Bennett
 * @author Gerd Knorr <kraxel@bytesex.org>
 * @author Ioana Patrascu
 * @author Tzvetan Horozov
 */

#ifndef IPCHECK_H
#define IPCHECK_H

/* we need size_t, and since it can be both unsigned int
   or unsigned long long, this IS platform dependent;
   but "stdlib.h" should be portable 'enough' to be
   unconditionally available... */
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


/* **************** constants ****************** */


/**
 * Just the version number of GNUnet-util implementation.
 * Encoded as
 * 0.6.1-4 => 0x00060104
 * 4.5.2   => 0x04050200
 *
 * Note that this version number is changed whenever
 * something changes GNUnet-util.  It does not have
 * to match exactly with the GNUnet version number;
 * especially the least significant bits may change
 * frequently, even between different SVN versions.
 */
#define GNUNET_UTIL_VERSION 0x00070004

/**
 * Named constants for return values.  The following
 * invariants hold: "NO == 0" (to allow "if (NO)")
 * "OK != SYSERR", "OK != NO", "NO != SYSERR"
 * and finally "YES != NO".
 */
#define OK      1
#define SYSERR -1
#define YES     1
#define NO      0


/* **************** structs ****************** */


/**
 * @brief an IPv4 address
 */
typedef struct {
  unsigned int addr; /* struct in_addr */
} IPaddr;

/**
 * @brief IPV4 network in CIDR notation.
 */
typedef struct CIDRNetwork {
  IPaddr network;
  IPaddr netmask;
} CIDRNetwork;

/**
 * @brief IPV4 network in CIDR notation.
 */
/*struct CIDRNetwork;*/

/**
 * @brief an IPV6 address.
 */
typedef struct {
  unsigned int addr[4]; /* struct in6_addr addr; */
} IP6addr;

/**
 * @brief network in CIDR notation for IPV6.
 */
typedef struct CIDR6Network {
  IP6addr network;
  IP6addr netmask;
} CIDR6Network;

/**
 * @brief IPV6 network in CIDR notation.
 */
/*struct CIDR6Network;*/


/* **************** Functions and Macros ************* */


/**
 * Parse a network specification. The argument specifies
 * a list of networks. The format is
 * <tt>[network/netmask;]*</tt> (no whitespace, must be terminated
 * with a semicolon). The network must be given in dotted-decimal
 * notation. The netmask can be given in CIDR notation (/16) or
 * in dotted-decimal (/255.255.0.0).
 * <p>
 * @param routeList a string specifying the forbidden networks
 * @return the converted list, NULL if the synatx is flawed
 */
struct CIDRNetwork * parseRoutes(const char * routeList);


/**
 * Check if the given IP address is in the list of
 * IP addresses.
 * @param list a list of networks
 * @param ip the IP to check (in network byte order)
 * @return NO if the IP is not in the list, YES if it it is
 */
int checkIPListed(const struct CIDRNetwork * list,
		  IPaddr ip);

#if 0
/**
 * Check if the given IP address is in the list of
 * IP addresses.
 * @param list a list of networks
 * @param ip the IP to check (in network byte order)
 * @return NO if the IP is not in the list, YES if it it is
 */
int checkIP6Listed(const struct CIDR6Network * list,
		   const IP6addr * ip);

/**
 * Parse a network specification. The argument specifies
 * a list of networks. The format is
 * <tt>[network/netmask;]*</tt> (no whitespace, must be terminated
 * with a semicolon). The network must be given in dotted-decimal
 * notation. The netmask can be given in CIDR notation (/16) or
 * in dotted-decimal (/255.255.0.0).
 * <p>
 * @param routeList a string specifying the forbidden networks
 * @return the converted list, NULL if the synatx is flawed
 */
struct CIDR6Network * parseRoutes6(const char * routeList);
#endif	/* #if 0 */

#ifdef __cplusplus
}
#endif


/* ifndef IPCHECK_H */
#endif
