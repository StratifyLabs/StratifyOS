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
#include "sos/dev/crypt.h"
#include "sos/api/crypt_api.h"


typedef struct {
	int fd;
	u32 o_key_flags;
	u32 mode;
	u8 key[32];
	u8 iv[16];
	u8 key_bits;
} device_aes_context_t;

static int update_mode(
		device_aes_context_t * context,
		u32 mode,
		const unsigned char * iv
		){
	if( context->mode != mode ){
		context->mode	= mode;

		crypt_attr_t attributes;
		if( iv != 0 ){
			memcpy(context->iv, iv, 16);
		}

		attributes.iv = context->iv;
		attributes.key = context->key;
		attributes.header_size = 0;
		attributes.o_flags =
				CRYPT_FLAG_SET_CIPHER |
				CRYPT_FLAG_SET_MODE |
				context->o_key_flags |
				mode;

		return ioctl(
					context->fd,
					I_CRYPT_SETATTR,
					&attributes
					);
	}
	return 0;
}

static int crypto_transaction(
		device_aes_context_t * context,
		const void * source,
		u8 * destination,
		u32 length
		){
	struct aiocb aio_operation;
	aio_operation.aio_buf = destination;
	aio_operation.aio_fildes = context->fd;
	aio_operation.aio_nbytes = length;
	aio_operation.aio_lio_opcode = LIO_READ;
	aio_operation.aio_offset = 0;
	aio_read(&aio_operation);
	return write(context->fd, source, length);
}

int device_aes_init(void ** context){
	int fd = open("/dev/crypt0", O_RDWR);
	if( fd < 0 ){
		return -1;
	}

	device_aes_context_t * c = malloc(sizeof(device_aes_context_t));
	if( c == 0 ){
		close(fd);
		return -1;
	}

	memset(c, 0, sizeof(device_aes_context_t));
	c->fd = fd;

	*context = c;
	return 0;
}

void device_aes_deinit(void ** context){
	if( *context != 0 ){
		void * c_ptr = *context;
		device_aes_context_t * c = c_ptr;
		close(c->fd);
		//zero out the keys
		memset(c, 0, sizeof(device_aes_context_t));
		c->fd = -1;
		*context = 0;
		free(c_ptr);
	}
}

int device_aes_set_key(
		void * context,
		const unsigned char * key,
		u32 keybits,
		u32 bits_per_word
		){
	if( context == 0 ){
		errno= EINVAL;
		return -1*__LINE__;
	}

	device_aes_context_t * c = context;
	if( keybits / 8 > 32 ){
		errno = EINVAL;
		return -1*__LINE__;
	}

	c->o_key_flags = 0;
	if( bits_per_word == 1 ){
		c->o_key_flags = CRYPT_FLAG_IS_DATA_1;
	} else if( bits_per_word == 8 ){
		c->o_key_flags = CRYPT_FLAG_IS_DATA_8;
	} else if( bits_per_word == 16 ){
		c->o_key_flags = CRYPT_FLAG_IS_DATA_16;
	} else {
		c->o_key_flags = CRYPT_FLAG_IS_DATA_32;
	}

	c->key_bits = keybits;
	if( keybits == 256 ){
		c->o_key_flags |= CRYPT_FLAG_IS_AES_256;
	} else if( keybits == 192 ){
		c->o_key_flags |= CRYPT_FLAG_IS_AES_192;
	} else {
		c->key_bits = 128;
		c->o_key_flags |= CRYPT_FLAG_IS_AES_128;
	}

	memset(c->key, 0, 32);
	memcpy(c->key, key, keybits/8);
	return 0;
}

int device_aes_encrypt_ecb(
		void * context,
		const unsigned char input[16],
unsigned char output[16]
){
	if( update_mode(
				context,
				CRYPT_FLAG_IS_AES_ECB |
				CRYPT_FLAG_IS_ENCRYPT,
				0
				) < 0 ){
		return -1;
	}

	return crypto_transaction(
				context,
				input,
				output,
				16
				);
}

int device_aes_decrypt_ecb(
		void * context,
		const unsigned char input[16],
unsigned char output[16]){
	if( update_mode(
				context,
				CRYPT_FLAG_IS_AES_ECB |
				CRYPT_FLAG_IS_DECRYPT,
				0
				) < 0 ){
		return -1;
	}

	return crypto_transaction(
				context,
				input,
				output,
				16
				);
}

int device_aes_encrypt_cbc(
		void * context,
		u32 length,
		unsigned char iv[16],
const unsigned char *input,
unsigned char *output ){

	if( update_mode(
				context,
				CRYPT_FLAG_IS_AES_CBC |
				CRYPT_FLAG_IS_ENCRYPT,
				iv
				) < 0 ){
		return -1;
	}

	return crypto_transaction(
				context,
				input,
				output,
				length
				);

	return 0;
}

int device_aes_decrypt_cbc(
		void * context,
		u32 length,
		unsigned char iv[16],
const unsigned char *input,
unsigned char *output ){

	if( update_mode(
				context,
				CRYPT_FLAG_IS_AES_CBC |
				CRYPT_FLAG_IS_DECRYPT,
				iv
				) < 0 ){
		return -1;
	}

	return crypto_transaction(
				context,
				input,
				output,
				length
				);

	return 0;
}

int device_aes_encrypt_ctr(
		void * context,
		u32 length,
		u32 *nc_off,
		unsigned char nonce_counter[16],
unsigned char stream_block[16],
const unsigned char *input,
unsigned char *output){
	return -1;
}

int device_aes_decrypt_ctr(
		void * context,
		u32 length,
		u32 *nc_off,
		unsigned char nonce_counter[16],
unsigned char stream_block[16],
const unsigned char *input,
unsigned char *output){
	return -1;
}

const crypt_aes_api_t device_aes_api = {
	.sos_api = {
		.name = "crypt_aes_device",
		.version = 0x0001,
		.git_hash = SOS_GIT_HASH
	},
	.init = device_aes_init,
	.deinit = device_aes_deinit,
	.set_key = device_aes_set_key,
	.encrypt_ecb = device_aes_encrypt_ecb,
	.decrypt_ecb = device_aes_decrypt_ecb,
	.encrypt_cbc = device_aes_encrypt_cbc,
	.decrypt_cbc = device_aes_decrypt_cbc,
	.encrypt_ctr = device_aes_encrypt_ctr,
	.decrypt_ctr = device_aes_decrypt_ctr
};


