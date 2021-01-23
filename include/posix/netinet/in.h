// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef POSIX_NETINET_IN_H_
#define POSIX_NETINET_IN_H_

/* Include the LWIP sockets header
 *
 * If LWIP isn't installed in the SDK, any apps that use netdb will get an error.
 *
 */
#if SOS_BOOTSTRAP_SOCKETS == 0
#include <lwip/netdb.h>
#endif


#endif /* POSIX_NETINET_IN_H_ */
