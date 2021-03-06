// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <unistd.h>

#include "../scheduler/scheduler_local.h"
#include "../unistd/unistd_local.h"
#include "sos/sos.h"
#include "sys/socket.h"

// int close(int s);
// int ioctl(int s, long cmd, void *argp);
// int fcntl(int s, int cmd, int val);
// int read(int s, void *mem, size_t len);
// int write(int s, const void *dataptr, size_t size);

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
  if (FILDES_IS_SOCKET(s)) {
    int accept_socket = SOS_SOCKET_API()->accept(s & ~FILDES_SOCKET_FLAG, addr, addrlen);
    if (accept_socket < 0) {
      return -1;
    }
    accept_socket |= FILDES_SOCKET_FLAG;
    return accept_socket;
  }
  errno = ENOTSOCK;
  return -1;
}

int bind(int s, const struct sockaddr *name, socklen_t namelen) {
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->bind(s & ~FILDES_SOCKET_FLAG, name, namelen);
  }
  errno = ENOTSOCK;
  return -1;
}

int shutdown(int s, int how) {
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->shutdown(s & ~FILDES_SOCKET_FLAG, how);
  }
  errno = ENOTSOCK;
  return -1;
}

int getpeername(int s, struct sockaddr *name, socklen_t *namelen) {
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->getpeername(s & ~FILDES_SOCKET_FLAG, name, namelen);
  }
  errno = ENOTSOCK;
  return -1;
}

int getsockname(int s, struct sockaddr *name, socklen_t *namelen) {
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->getsockname(s & ~FILDES_SOCKET_FLAG, name, namelen);
  }
  errno = ENOTSOCK;
  return -1;
}

int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen) {
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->getsockopt(
      s & ~FILDES_SOCKET_FLAG, level, optname, optval, optlen);
  }
  errno = ENOTSOCK;
  return -1;
}

int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen) {
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->setsockopt(
      s & ~FILDES_SOCKET_FLAG, level, optname, optval, optlen);
  }
  errno = ENOTSOCK;
  return -1;
}

int connect(int s, const struct sockaddr *name, socklen_t namelen) {
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->connect(s & ~FILDES_SOCKET_FLAG, name, namelen);
  }
  errno = ENOTSOCK;
  return -1;
}

int listen(int s, int backlog) {
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->listen(s & ~FILDES_SOCKET_FLAG, backlog);
  }
  errno = ENOTSOCK;
  return -1;
}

int recv(int s, void *mem, size_t len, int flags) {
  scheduler_check_cancellation();
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->recv(s & ~FILDES_SOCKET_FLAG, mem, len, flags);
  }
  errno = ENOTSOCK;
  return -1;
}

int recvfrom(
  int s,
  void *mem,
  size_t len,
  int flags,
  struct sockaddr *from,
  socklen_t *fromlen) {
  scheduler_check_cancellation();
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->recvfrom(
      s & ~FILDES_SOCKET_FLAG, mem, len, flags, from, fromlen);
  }
  errno = ENOTSOCK;
  return -1;
}

int send(int s, const void *dataptr, size_t size, int flags) {
  scheduler_check_cancellation();
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->send(s & ~FILDES_SOCKET_FLAG, dataptr, size, flags);
  }
  errno = ENOTSOCK;
  return -1;
}

int sendmsg(int s, const struct msghdr *message, int flags) {
  scheduler_check_cancellation();
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->sendmsg(s & ~FILDES_SOCKET_FLAG, message, flags);
  }
  errno = ENOTSOCK;
  return -1;
}

int sendto(
  int s,
  const void *dataptr,
  size_t size,
  int flags,
  const struct sockaddr *to,
  socklen_t tolen) {
  scheduler_check_cancellation();
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->sendto(
      s & ~FILDES_SOCKET_FLAG, dataptr, size, flags, to, tolen);
  }
  errno = ENOTSOCK;
  return -1;
}

int socket(int domain, int type, int protocol) {
  int s;
  s = SOS_SOCKET_API()->socket(domain, type, protocol);
  if (s < 0) {
    return -1;
  }
  return s | FILDES_SOCKET_FLAG;
}

int writev(int s, const struct iovec *iov, int iovcnt) {
  scheduler_check_cancellation();
  if (FILDES_IS_SOCKET(s)) {
    return SOS_SOCKET_API()->writev(s & ~FILDES_SOCKET_FLAG, iov, iovcnt);
  }
  errno = ENOTSOCK;
  return -1;
}

int select(
  int maxfdp1,
  fd_set *readset,
  fd_set *writeset,
  fd_set *exceptset,
  struct timeval *timeout) {
  scheduler_check_cancellation();
  return SOS_SOCKET_API()->select(maxfdp1, readset, writeset, exceptset, timeout);
}

struct hostent *gethostbyname(const char *name) {
  return SOS_SOCKET_API()->gethostbyname(name);
}

int gethostbyname_r(
  const char *name,
  struct hostent *ret,
  char *buf,
  size_t buflen,
  struct hostent **result,
  int *h_errnop) {
  return SOS_SOCKET_API()->gethostbyname_r(name, ret, buf, buflen, result, h_errnop);
}

void freeaddrinfo(struct addrinfo *ai) { SOS_SOCKET_API()->freeaddrinfo(ai); }

int getaddrinfo(
  const char *nodename,
  const char *servname,
  const struct addrinfo *hints,
  struct addrinfo **res) {
  return SOS_SOCKET_API()->getaddrinfo(nodename, servname, hints, res);
}

in_addr_t inet_addr(const char *cp) { return SOS_SOCKET_API()->inet_addr(cp); }

char *inet_ntoa(struct in_addr in) { return SOS_SOCKET_API()->inet_ntoa(in); }

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size) {
  return SOS_SOCKET_API()->inet_ntop(af, src, dst, size);
}

int inet_pton(int af, const char *src, void *dst) {
  return SOS_SOCKET_API()->inet_pton(af, src, dst);
}

uint32_t htonl(uint32_t hostlong) { return __REV(hostlong); }

uint16_t htons(uint16_t hostshort) { return (uint16_t)__REVSH(hostshort); }

uint32_t ntohl(uint32_t netlong) { return __REV(netlong); }

uint16_t ntohs(uint16_t netshort) { return (uint16_t)__REVSH(netshort); }
