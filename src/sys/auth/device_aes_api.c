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
#include "sos/crypt_api.h"


typedef struct {
	int fd;
	u32 o_flags;
	u8 key[32];
	u8 key_bits;
} device_aes_context_t;

int device_aes_init(void ** context){
	int fd = open("/dev/crypt0", O_RDWR);
	if( fd < 0 ){
		return -1;
	}

	void * c = malloc(sizeof(device_aes_context_t));
	if( c == 0 ){
		close(fd);
		return -1;
	}

	((device_aes_context_t*)c)->fd = fd;
	((device_aes_context_t*)c)->o_flags = 0;

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

	c->o_flags &= ~(
				CRYPT_FLAG_IS_DATA_1 |
				CRYPT_FLAG_IS_DATA_8 |
				CRYPT_FLAG_IS_DATA_16 |
				CRYPT_FLAG_IS_DATA_32
				);

	if( bits_per_word == 1 ){	c->o_flags = CRYPT_FLAG_IS_DATA_1; }
	else if( bits_per_word == 8 ){	c->o_flags = CRYPT_FLAG_IS_DATA_8; }
	else if( bits_per_word == 16 ){	c->o_flags = CRYPT_FLAG_IS_DATA_16; }
	else {	c->o_flags = CRYPT_FLAG_IS_DATA_32; }

	c->key_bits = keybits;
	memcpy(c->key, key, keybits/8);
	return 0;
}

int device_aes_encrypt_ecb(
		void * context,
		const unsigned char input[16],
unsigned char output[16]
){
	u32 mode =
			CRYPT_FLAG_IS_AES_ECB |
			CRYPT_FLAG_IS_ENCRYPT;
	device_aes_context_t * c = context;
	if( (c->o_flags & mode) != mode ){
		crypt_attr_t attributes;
		attributes.o_flags = c->o_flags;

	}
	return -1;

}

int device_aes_decrypt_ecb(
		void * context,
		const unsigned char input[16],
unsigned char output[16]){
	u32 mode =
			CRYPT_FLAG_IS_AES_ECB |
			CRYPT_FLAG_IS_DECRYPT;
	device_aes_context_t * c = context;
	if( (c->o_flags & mode) != mode ){

	}
	return -1;
}

int device_aes_encrypt_cbc(
		void * context,
		u32 length,
		unsigned char iv[16],
const unsigned char *input,
unsigned char *output ){
	u32 mode =
			CRYPT_FLAG_IS_AES_CBC |
			CRYPT_FLAG_IS_ENCRYPT;
	device_aes_context_t * c = context;
	if( (c->o_flags & mode) != mode ){

	}
	return -1;
}

int device_aes_decrypt_cbc(
		void * context,
		u32 length,
		unsigned char iv[16],
const unsigned char *input,
unsigned char *output ){
	u32 mode =
			CRYPT_FLAG_IS_AES_CBC |
			CRYPT_FLAG_IS_DECRYPT;
	device_aes_context_t * c = context;
	if( (c->o_flags & mode) != mode ){

	}
	return -1;
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


