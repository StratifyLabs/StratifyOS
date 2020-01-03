#ifndef SOS_CRYPT_API_H
#define SOS_CRYPT_API_H

#include "mcu/types.h"

typedef struct {
	int (*init)(void ** context);
	void (*deinit)(void ** context);
	int (*set_key)(void * context, const unsigned char * key, u32 keybits);

	int (*encrypt_ecb)(void * context,
							  const unsigned char input[16],
							  unsigned char output[16]);

	int (*decrypt_ecb)(void * context,
							  const unsigned char input[16],
							  unsigned char output[16]);

	int (*encrypt_cbc)(void * context,
							  u32 length,
							  unsigned char iv[16],
							  const unsigned char *input,
							  unsigned char *output );

	int (*decrypt_cbc)(void * context,
							  u32 length,
							  unsigned char iv[16],
							  const unsigned char *input,
							  unsigned char *output );

	int (*encrypt_ctr)(void * context,
								  u32 length,
								  u32 *nc_off,
								  unsigned char nonce_counter[16],
								  unsigned char stream_block[16],
								  const unsigned char *input,
								  unsigned char *output);

	int (*decrypt_ctr)(void * context,
								  u32 length,
								  u32 *nc_off,
								  unsigned char nonce_counter[16],
								  unsigned char stream_block[16],
								  const unsigned char *input,
								  unsigned char *output);

} crypt_aes_api_t;


//Can be used for SHA256
typedef struct {
	int (*init)(void ** context);
	void (*deinit)(void ** context);
	int (*start)(void * context);
	int (*update)(void * context, const unsigned char * input, u32 size);
	int (*finish)(void * context, unsigned char * output, u32 size);
} crypt_hash_api_t;

typedef struct {
	int (*init)(void ** context);
	void (*deinit)(void ** context);
} crypt_ecc_api_t;

typedef struct {
	//init
	//parse public key
	//encrypt using public key
	//parse key (private)
	//get key type
} crypt_public_key_api_t;

typedef struct {
	void (*init)(void ** context, int padding, int hash_id);
	void (*deinit)(void ** context);

	//generate a key
	//decrypt
} crypt_rsa_api_t;

typedef struct {
	int (*init)(void ** context);
	void (*deinit)(void ** context);
	int (*seed)(void * context, const unsigned char * data, u32 data_len);
	int (*random)(void * context, unsigned char * output, u32 output_length);
} crypt_random_api_t;


#if !defined __link
#define CRYPT_SHA256_API_REQUEST MCU_API_REQUEST_CODE('s','2','5','6')
#define CRYPT_SHA512_API_REQUEST MCU_API_REQUEST_CODE('s','5','1','2')
#define CRYPT_RANDOM_API_REQUEST MCU_API_REQUEST_CODE('r','a','n','d')
#define CRYPT_AES_API_REQUEST MCU_API_REQUEST_CODE('a','e','s','!')
#endif

extern const crypt_hash_api_t tinycrypt_sha256_hash_api;
extern const crypt_hash_api_t device_sha256_hash_api;
extern const crypt_aes_api_t device_aes_api;

#endif // SOS_CRYPT_API_H
