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

#include <unistd.h>
#include "sos/sos.h"
#include "sys/socket.h"
#include "../unistd/unistd_local.h"

//int close(int s);
//int ioctl(int s, long cmd, void *argp);
//int fcntl(int s, int cmd, int val);
//int read(int s, void *mem, size_t len);
//int write(int s, const void *dataptr, size_t size);


int accept(int s, struct sockaddr *addr, socklen_t *addrlen){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->accept(s & ~FILDES_SOCKET_FLAG, addr, addrlen);
	}
	errno = ENOTSOCK;
	return -1;
}

int bind(int s, const struct sockaddr *name, socklen_t namelen){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->bind(s & ~FILDES_SOCKET_FLAG, name, namelen);
	}
	errno = ENOTSOCK;
	return -1;
}

int shutdown(int s, int how){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->shutdown(s & ~FILDES_SOCKET_FLAG, how);
	}
	errno = ENOTSOCK;
	return -1;
}

int getpeername(int s, struct sockaddr *name, socklen_t *namelen){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->getpeername(s & ~FILDES_SOCKET_FLAG, name, namelen);
	}
	errno = ENOTSOCK;
	return -1;
}


int getsockname(int s, struct sockaddr *name, socklen_t *namelen){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->getsockname(s & ~FILDES_SOCKET_FLAG, name, namelen);
	}
	errno = ENOTSOCK;
	return -1;
}


int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->getsockopt(s & ~FILDES_SOCKET_FLAG, level, optname, optval, optlen);
	}
	errno = ENOTSOCK;
	return -1;
}


int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->setsockopt(s & ~FILDES_SOCKET_FLAG, level, optname, optval, optlen);
	}
	errno = ENOTSOCK;
	return -1;
}


int connect(int s, const struct sockaddr *name, socklen_t namelen){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->connect(s & ~FILDES_SOCKET_FLAG, name, namelen);
	}
	errno = ENOTSOCK;
	return -1;
}


int listen(int s, int backlog){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->listen(s & ~FILDES_SOCKET_FLAG, backlog);
	}
	errno = ENOTSOCK;
	return -1;
}


int recv(int s, void *mem, size_t len, int flags){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->recv(s & ~FILDES_SOCKET_FLAG, mem, len, flags);
	}
	errno = ENOTSOCK;
	return -1;
}


int recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->recvfrom(s & ~FILDES_SOCKET_FLAG, mem, len, flags, from, fromlen);
	}
	errno = ENOTSOCK;
	return -1;
}


int send(int s, const void *dataptr, size_t size, int flags){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->send(s & ~FILDES_SOCKET_FLAG, dataptr, size, flags);
	}
	errno = ENOTSOCK;
	return -1;
}

int sendmsg(int s, const struct msghdr *message, int flags){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->sendmsg(s & ~FILDES_SOCKET_FLAG, message, flags);
	}
	errno = ENOTSOCK;
	return -1;
}

int sendto(int s, const void *dataptr, size_t size, int flags,
			  const struct sockaddr *to, socklen_t tolen){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->sendto(s & ~FILDES_SOCKET_FLAG, dataptr, size, flags, to, tolen);
	}
	errno = ENOTSOCK;
	return -1;
}

int socket(int domain, int type, int protocol){
	int s;
	s = sos_board_config.socket_api->socket(domain, type, protocol);
	if( s < 0 ){
		return -1;
	}
	return s | FILDES_SOCKET_FLAG;
}


int writev(int s, const struct iovec *iov, int iovcnt){
	if( s & FILDES_SOCKET_FLAG ){
		return sos_board_config.socket_api->writev(s & ~FILDES_SOCKET_FLAG, iov, iovcnt);
	}
	errno = ENOTSOCK;
	return -1;
}


int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout){
	return sos_board_config.socket_api->select(maxfdp1, readset, writeset, exceptset, timeout);
}

struct hostent * gethostbyname(const char *name){
	return sos_board_config.socket_api->gethostbyname(name);
}

int gethostbyname_r(const char *name, struct hostent *ret, char *buf, size_t buflen, struct hostent **result, int *h_errnop){
	return sos_board_config.socket_api->gethostbyname_r(name, ret, buf, buflen, result, h_errnop);
}

void freeaddrinfo(struct addrinfo *ai){
	sos_board_config.socket_api->freeaddrinfo(ai);
}

int getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res){
	return sos_board_config.socket_api->getaddrinfo(nodename, servname, hints, res);
}

