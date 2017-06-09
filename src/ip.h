#ifndef IP_H
#define IP_H

int
cidr4_match(const struct in_addr *addr,
            const struct in_addr *net,
            int bits
);

int
ipv4_is_in_net(const struct in_addr *addr,       /* host byte order */
               const struct in_addr *netaddr,
               const struct in_addr *netmask
);

#endif /* ifndef IP_H */
