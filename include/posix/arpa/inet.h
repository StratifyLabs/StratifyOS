// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef POSIX_ARPA_INET_H_
#define POSIX_ARPA_INET_H_

/* Include the LWIP sockets header
 *
 * If LWIP isn't installed in the SDK, any apps that use netdb will get an error.
 *
 */

#if SOS_BOOTSTRAP_SOCKETS
#include <sdk/types.h>
typedef u32 in_addr_t;
typedef u16 in_port_t;
#define INET_ADDRSTRLEN 8
#define INET6_ADDRSTRLEN 8
#else
#include <lwip/sockets.h>
#endif //SOS_BOOTSTRAP_SOCKETS


#if defined __cplusplus
extern "C" {
#endif

#if defined inet_addr
#undef inet_addr
#endif

#if defined inet_ntoa
#undef inet_ntoa
#endif

#if defined inet_ntop
#undef inet_ntop
#endif

#if defined inet_pton
#undef inet_pton
#endif

#if defined htonl
#undef htonl
#endif

#if defined htons
#undef htons
#endif

#if defined ntohl
#undef ntohl
#endif

#if defined ntohs
#undef ntohs
#endif

in_addr_t inet_addr(const char * cp);
char * inet_ntoa(struct in_addr in);
const char * inet_ntop(int af, const void * src, char * dst, socklen_t size);
int inet_pton(int af, const char * src, void * dst);

uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);

#if defined __cplusplus
}
#endif


#endif /* POSIX_ARPA_INET_H_ */
