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

#if HAVE_LWIP_SOCKETS_H

int accept(int s, struct sockaddr *addr, socklen_t *addrlen){
	return sos_board_config.socket_api->accept(s & ~FILDES_SOCKET_FLAG, addr, addrlen);
}

int bind(int s, const struct sockaddr *name, socklen_t namelen){
	return sos_board_config.socket_api->bind(s & ~FILDES_SOCKET_FLAG, name, namelen);
}

int shutdown(int s, int how){
	return sos_board_config.socket_api->shutdown(s & ~FILDES_SOCKET_FLAG, how);
}

int getpeername(int s, struct sockaddr *name, socklen_t *namelen){
	return sos_board_config.socket_api->getpeername(s & ~FILDES_SOCKET_FLAG, name, namelen);
}


int getsockname(int s, struct sockaddr *name, socklen_t *namelen){
	return sos_board_config.socket_api->getsockname(s & ~FILDES_SOCKET_FLAG, name, namelen);
}


int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen){
	return sos_board_config.socket_api->getsockopt(s & ~FILDES_SOCKET_FLAG, level, optname, optval, optlen);
}


int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen){
	return sos_board_config.socket_api->setsockopt(s & ~FILDES_SOCKET_FLAG, level, optname, optval, optlen);
}


int connect(int s, const struct sockaddr *name, socklen_t namelen){
	return sos_board_config.socket_api->connect(s & ~FILDES_SOCKET_FLAG, name, namelen);
}


int listen(int s, int backlog){
	return sos_board_config.socket_api->listen(s & ~FILDES_SOCKET_FLAG, backlog);
}


int recv(int s, void *mem, size_t len, int flags){
	return sos_board_config.socket_api->recv(s & ~FILDES_SOCKET_FLAG, mem, len, flags);
}


int recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen){
	return sos_board_config.socket_api->recvfrom(s & ~FILDES_SOCKET_FLAG, mem, len, flags, from, fromlen);
}


int send(int s, const void *dataptr, size_t size, int flags){
	return sos_board_config.socket_api->send(s & ~FILDES_SOCKET_FLAG, dataptr, size, flags);
}


int sendmsg(int s, const struct msghdr *message, int flags){
	return sos_board_config.socket_api->sendmsg(s & ~FILDES_SOCKET_FLAG, message, flags);
}


int sendto(int s, const void *dataptr, size_t size, int flags,
		const struct sockaddr *to, socklen_t tolen){
	return sos_board_config.socket_api->sendto(s & ~FILDES_SOCKET_FLAG, dataptr, size, flags, to, tolen);
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
	return sos_board_config.socket_api->writev(s & ~FILDES_SOCKET_FLAG, iov, iovcnt);
}


int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout){
	return sos_board_config.socket_api->select(maxfdp1, readset, writeset, exceptset, timeout);
}

const sos_socket_api_t lwip_socket_api = {
		.accept = lwip_accept,
		.bind = lwip_bind,
		.shutdown = lwip_shutdown,
		.getpeername = lwip_getpeername,
		.getsockname = lwip_getsockname,
		.getsockopt = lwip_getsockopt,
		.setsockopt = lwip_setsockopt,
		.close = lwip_close,
		.connect = lwip_connect,
		.read = lwip_read,
		.listen = lwip_listen,
		.recv = lwip_recv,
		.recvfrom = lwip_recvfrom,
		.send = lwip_send,
		.sendmsg = lwip_sendmsg,
		.sendto = lwip_sendto,
		.socket = lwip_socket,
		.write = lwip_write,
		.writev = lwip_writev,
		.select = lwip_select,
		.ioctl = lwip_ioctl,
		.fcntl = lwip_fcntl
};

#endif
