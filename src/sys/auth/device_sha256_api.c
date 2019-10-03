/* Copyright 2011-2019 Tyler Gilbert;
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


#include "config.h"
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "sos/dev/hash.h"
#include "device/auth.h"
#include "sos/crypt_api.h"


typedef struct {
	int fd;
	u8 digest[32];
} device_sha256_context_t;

int device_sha256_init(void ** context){
	int fd = open("/dev/hash0", O_RDWR);
	if( fd < 0 ){
		return -1;
	}

	void * c = malloc(sizeof(device_sha256_context_t));
	if( c == 0 ){
		close(fd);
		return -1;
	}

	((device_sha256_context_t*)c)->fd = fd;

	*context = c;
	return 0;
}

void device_sha256_deinit(void ** context){
	if( *context != 0 ){
		void * c_ptr = *context;
		device_sha256_context_t * c = c_ptr;
		close(c->fd);
		c->fd = -1;
		*context = 0;
		free(c_ptr);
	}
}

int device_sha256_start(void * context){
	//device_sha256_context_t * c = context;
	//use dev/hash to start a new hash ioctl to re-init

	return 0;
}

int device_sha256_update(void * context, const unsigned char * input, u32 size){
	device_sha256_context_t * c = context;
	int result = write(c->fd, input, size);
	if( result < 0 ){
		return result;
	}

	return 0;
}

int device_sha256_finish(void * context, unsigned char * output, u32 size){
	device_sha256_context_t * c = context;

	if( size < 32 ){
		errno = EINVAL;
		return -1;
	}

	int result = read(c->fd, output, 32);
	if( result < 0 ){
		return result;
	}

	return 0;
}

const crypt_hash_api_t device_sha256_hash_api = {
	.init = device_sha256_init,
	.deinit = device_sha256_deinit,
	.start = device_sha256_start,
	.update = device_sha256_update,
	.finish = device_sha256_finish
};


