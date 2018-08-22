/* Copyright 2011-2017 Tyler Gilbert;
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef POSIX_ARPA_INET_H_
#define POSIX_ARPA_INET_H_

/* Include the LWIP sockets header
 *
 * If LWIP isn't installed in the SDK, any apps that use netdb will get an error.
 *
 */

#if !defined SOS_BOOTSTRAP_SOCKETS
#include <lwip/sockets.h>
#else
#include <mcu/types.h>
typedef u32 in_addr_t;
typedef u16 in_port_t;
#define INET_ADDRSTRLEN 8
#define INET6_ADDRSTRLEN 8

//defs copied from LWIP
#define htons(x) ((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))
#define ntohs(x) PP_HTONS(x)
#define htonl(x) ((((x) & 0x000000ffUL) << 24) | \
                     (((x) & 0x0000ff00UL) <<  8) | \
                     (((x) & 0x00ff0000UL) >>  8) | \
                     (((x) & 0xff000000UL) >> 24))
#define ntohl(x) PP_HTONL(x)

//these shoudl be part of the Socket API if they are to be called from applications
#define inet_addr(cp)                   ipaddr_addr(cp)
#define inet_aton(cp, addr)             ip4addr_aton(cp, (ip4_addr_t*)addr)
#define inet_ntoa(addr)                 ip4addr_ntoa((const ip4_addr_t*)&(addr))
#define inet_ntoa_r(addr, buf, buflen)  ip4addr_ntoa_r((const ip4_addr_t*)&(addr), buf, buflen)


#endif //SOS_BOOTSTRAP_SOCKETS


#endif /* POSIX_ARPA_INET_H_ */
