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
#include "device/auth.h"
#include "sos/crypt_api.h"

#include "tinycrypt/constants.h"
#include "tinycrypt/sha256.h"

int tinycrypt_sha256_init(void ** context){
	void * c = malloc(sizeof(struct tc_sha256_state_struct));
	if( c == 0 ){
		return -1;
	}
	memset(c, 0, sizeof(struct tc_sha256_state_struct));
	*context = c;
	return 0;
}

void tinycrypt_sha256_deinit(void ** context){
	if( *context != 0 ){
		void * c = *context;
		*context = 0;
		free(c);
	}
}

int tinycrypt_sha256_start(void * context){
	if( tc_sha256_init(context) == TC_CRYPTO_FAIL ){
		return -1;
	}
	return 0;
}

int tinycrypt_sha256_update(void * context, const unsigned char * input, u32 size){
	if( tc_sha256_update(context, (const u8*)input, size) == TC_CRYPTO_FAIL ){
		return -1;
	}
	return 0;
}

int tinycrypt_sha256_finish(void * context, unsigned char * output, u32 size){
	if( size != 32 ){
		return -1;
	}
	if( tc_sha256_final(output, context) == TC_CRYPTO_FAIL ){
		return -1;
	}
	return 0;
}

const crypt_hash_api_t tinycrypt_sha256_hash_api = {
	.init = tinycrypt_sha256_init,
	.deinit = tinycrypt_sha256_deinit,
	.start = tinycrypt_sha256_start,
	.update = tinycrypt_sha256_update,
	.finish = tinycrypt_sha256_finish
};


