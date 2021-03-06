// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef POSIX_SYS_SOCKET_H_
#define POSIX_SYS_SOCKET_H_

/* Include the LWIP sockets header
 *
 * If LWIP isn't installed in the SDK, any apps that use sockets will get an error.
 *
 */

#if SOS_BOOTSTRAP_SOCKETS
#include <sdk/types.h>
#include <sys/time.h>
typedef u32 socklen_t;
typedef u32 fd_set;
struct sockaddr;
struct msghdr;
struct iovec;
struct in_addr {
  int dummy;
};
#else
#include <lwip/sockets.h>
#endif // SOS_BOOTSTRAP_SOCKETS

#include "arpa/inet.h"

#if defined __cplusplus
extern "C" {
#endif

// these functions are for sockets only
int accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int bind(int s, const struct sockaddr *name, socklen_t namelen);
int shutdown(int s, int how);
int getpeername(int s, struct sockaddr *name, socklen_t *namelen);
int getsockname(int s, struct sockaddr *name, socklen_t *namelen);
int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);
int connect(int s, const struct sockaddr *name, socklen_t namelen);
int listen(int s, int backlog);
int recv(int s, void *mem, size_t len, int flags);
int recvfrom(
  int s,
  void *mem,
  size_t len,
  int flags,
  struct sockaddr *from,
  socklen_t *fromlen);
int send(int s, const void *dataptr, size_t size, int flags);
int sendmsg(int s, const struct msghdr *message, int flags);
int sendto(
  int s,
  const void *dataptr,
  size_t size,
  int flags,
  const struct sockaddr *to,
  socklen_t tolen);
int socket(int domain, int type, int protocol);

// these functions are currently only for sockets but should be supported on non-sockets
// as well
int writev(int s, const struct iovec *iov, int iovcnt);
int select(
  int maxfdp1,
  fd_set *readset,
  fd_set *writeset,
  fd_set *exceptset,
  struct timeval *timeout);

// these are supported on both sockets and non-sockets
// int read(int s, void *mem, size_t len);
// int ioctl(int s, long cmd, void *argp);
// int fcntl(int s, int cmd, int val);
// int write(int s, const void *dataptr, size_t size);
// int close(int s);

#if defined __cplusplus
}
#endif

struct addrinfo;

#define SOS_SOCKET_API_T 1
typedef struct {
  int (*startup)(const void *);
  int (*accept)(int s, struct sockaddr *addr, socklen_t *addrlen);
  int (*bind)(int s, const struct sockaddr *name, socklen_t namelen);
  int (*shutdown)(int s, int how);
  int (*getpeername)(int s, struct sockaddr *name, socklen_t *namelen);
  int (*getsockname)(int s, struct sockaddr *name, socklen_t *namelen);
  int (*getsockopt)(int s, int level, int optname, void *optval, socklen_t *optlen);
  int (*setsockopt)(int s, int level, int optname, const void *optval, socklen_t optlen);
  int (*close)(int s);
  int (*connect)(int s, const struct sockaddr *name, socklen_t namelen);
  int (*listen)(int s, int backlog);
  int (*recv)(int s, void *mem, size_t len, int flags);
  int (*read)(int s, void *mem, size_t len);
  int (*recvfrom)(
    int s,
    void *mem,
    size_t len,
    int flags,
    struct sockaddr *from,
    socklen_t *fromlen);
  int (*send)(int s, const void *dataptr, size_t size, int flags);
  int (*sendmsg)(int s, const struct msghdr *message, int flags);
  int (*sendto)(
    int s,
    const void *dataptr,
    size_t size,
    int flags,
    const struct sockaddr *to,
    socklen_t tolen);
  int (*socket)(int domain, int type, int protocol);
  int (*write)(int s, const void *dataptr, size_t size);
  int (*writev)(int s, const struct iovec *iov, int iovcnt);
  int (*select)(
    int maxfdp1,
    fd_set *readset,
    fd_set *writeset,
    fd_set *exceptset,
    struct timeval *timeout);
  int (*ioctl)(int s, long cmd, void *argp);
  int (*fcntl)(int s, int cmd, int val);
  int (*fsync)(int s);
  struct hostent *(*gethostbyname)(const char *name);
  int (*gethostbyname_r)(
    const char *name,
    struct hostent *ret,
    char *buf,
    size_t buflen,
    struct hostent **result,
    int *h_errnop);
  void (*freeaddrinfo)(struct addrinfo *ai);
  int (*getaddrinfo)(
    const char *nodename,
    const char *servname,
    const struct addrinfo *hints,
    struct addrinfo **res);
  in_addr_t (*inet_addr)(const char *cp);
  char *(*inet_ntoa)(struct in_addr in);
  const char *(*inet_ntop)(int af, const void *src, char *dst, socklen_t size);
  int (*inet_pton)(int af, const char *src, void *dst);
  const void *config;
  void *state;
} sos_socket_api_t;

#define SOS_SOCKET_API() ((sos_socket_api_t *)sos_config.socket_api)

#endif /* POSIX_SYS_SOCKET_H_ */
