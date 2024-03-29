
#include <sdk/api.h>

#include <string.h>

#include "cortexm/task.h"
#include "device/auth.h"

#define DECLARE_KEY(key)                                                                 \
  volatile u8 key[32];                                                                   \
  __NOP();                                                                               \
  __NOP();                                                                               \
  __NOP();                                                                               \
  __NOP();                                                                               \
  key[0] = 0xa0;                                                                         \
  key[1] = 0xa1;                                                                         \
  key[2] = 0xa2;                                                                         \
  key[3] = 0xa3;                                                                         \
  key[4] = 0xa4;                                                                         \
  key[5] = 0xa5;                                                                         \
  key[6] = 0xa6;                                                                         \
  key[7] = 0xa7;                                                                         \
  key[8] = 0xa8;                                                                         \
  key[9] = 0xa9;                                                                         \
  key[10] = 0xaa;                                                                         \
  key[11] = 0xab;                                                                        \
  key[12] = 0xac;                                                                        \
  key[13] = 0xad;                                                                        \
  key[14] = 0xae;                                                                        \
  key[15] = 0xaf;                                                                        \
  key[16] = 0xb0;                                                                        \
  key[17] = 0xb1;                                                                        \
  key[18] = 0xb2;                                                                        \
  key[19] = 0xb3;                                                                        \
  key[20] = 0xb4;                                                                        \
  key[21] = 0xb5;                                                                        \
  key[22] = 0xb6;                                                                        \
  key[23] = 0xb7;                                                                        \
  key[24] = 0xb8;                                                                        \
  key[25] = 0xb9;                                                                        \
  key[26] = 0xba;                                                                        \
  key[27] = 0xbb;                                                                        \
  key[28] = 0xbc;                                                                        \
  key[29] = 0xbd;                                                                        \
  key[30] = 0xbe;                                                                        \
  key[31] = 0xbf

int auth_pure_code_calculate_authentication(
  auth_token_t *dest,
  const auth_token_t *input,
  int key_is_first) {

  DECLARE_KEY(private_key);

  if( sos_config.sys.kernel_request_api == NULL ){
    return -1;
  }

  const crypt_hash_api_t *sha256_api =
    sos_config.sys.kernel_request_api(CRYPT_SHA256_ROOT_API_REQUEST);

  if (sha256_api == NULL) {
    return -1;
  }

  u8 sha256_context[sha256_api->get_context_size()];
  void *context = sha256_context;
  sha256_api->init(&context);
  sha256_api->start(context);
  if (key_is_first) {
    sha256_api->update(context, (const u8 *)private_key, sizeof(auth_token_t));
    sha256_api->update(context, (const u8 *)input, sizeof(auth_token_t));
  } else {
    sha256_api->update(context, (const u8 *)input, sizeof(auth_token_t));
    sha256_api->update(context, (const u8 *)private_key, sizeof(auth_token_t));
  }
  sha256_api->finish(context, dest->data, sizeof(auth_token_t));
  sha256_api->deinit(&context);

  return 0;
}

int auth_pure_code_encrypt_decrypt(
  u8 *iv,
  const u8 *source,
  u8 *dest,
  size_t nbyte,
  int is_encrypt) {
  // sl will dynamically update the private key
  DECLARE_KEY(private_key);

  const crypt_aes_api_t *api =
    sos_config.sys.kernel_request_api(CRYPT_AES_ROOT_API_REQUEST);
  const size_t context_size = api->get_context_size();

  u8 context_buffer[context_size];
  void *context = context_buffer;
  api->init(&context);

  api->set_key(context, (const u8 *)private_key, 128, 8);
  if (is_encrypt) {
    const u8 *plain = source;
    u8 *cipher = dest;
    api->encrypt_cbc(context, nbyte, iv, plain, cipher);
  } else {
    const u8 *cipher = source;
    u8 *plain = dest;
    api->decrypt_cbc(context, nbyte, iv, cipher, plain);
  }

  api->deinit(&context);
  return 0;
}
