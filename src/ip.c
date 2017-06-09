#include "app.h"

#include <stdio.h>
#include <string.h>

/* network types */
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif

#include "ip.h"

/**
 * Check if the IP address is in the network.
 * https://stackoverflow.com/a/25290862
 * @param addr IP address
 * @param net network
 * @param bits cidr length
 * @return 1 if the IP address is in the network, otherwise 0
 */
int
cidr4_match(const struct in_addr *addr,
            const struct in_addr *net,
            int bits
)
{
    /* TODO: change bits to uint8_t after testing cross-platform */
    if (bits == 0) {
        /* C99 6.5.7 (3): u32 << 32 is undefined behaviour */
        return 1;
    }
    /* s_addr is always in network (big endian) byte order on all platforms */
    if ((addr->s_addr ^ net->s_addr) & htonl(0xFFFFFFFFu << (32 - bits))) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * Check if the IP address is in the network.
 * @param addr IP address (host byte order)
 * @param netaddr network
 * @param netmask network mask (in network byte order)
 * @return 1 if the IP address is in the network, otherwise 0
 */
int
ipv4_is_in_net(const struct in_addr *addr,      /* host byte order */
               const struct in_addr *netaddr,
               const struct in_addr *netmask
)
{
    /* s_addr is always in network (big endian) byte order on all platforms */
    if ((addr->s_addr & netmask->s_addr) == (netaddr->s_addr & netmask->s_addr)) {
        return 1;
    } else {
        return 0;
    }
}
