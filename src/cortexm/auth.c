
#include <string.h>

#include "sos/config.h"
#include "device/auth.h"

#include "sos_config.h"

static const u8 *get_public_key() {
  return (const u8 *)((u32)sos_config.sys.secret_key_address & ~0x01);
}

int cortexm_auth_handle_event(int event, void *args) {
#if CONFIG_BOOT_IS_VERIFY_SIGNATURE
  if (event == BOOTLOADER_EVENT_AUTHENTICATE) {
    bootloader_event_authenication_t *auth = args;
    auth_token_t input;
    auth_token_t result;
    memcpy(input.data, auth->auth_data, 32);
    const int auth_result =
      auth_pure_code_calculate_authentication(&result, &input, auth->is_key_first);
    if (auth_result < 0) {
      return auth_result;
    }

    memcpy(auth->result, result.data, 32);
    return 0;
  }

  if (event == BOOTLOADER_EVENT_GET_PUBLIC_KEY) {
    auth_public_key_t *key = args;
    memcpy(key, get_public_key(), sizeof(auth_public_key_t));
    return 0;
  }
#endif

#if CONFIG_BOOT_IS_AES_CRYPTO
  if (event == BOOTLOADER_EVENT_ENCRYPT) {
    bootloader_event_crypto_t *crypto = args;
    const int crypto_result = auth_pure_code_encrypt_decrypt(
      crypto->iv, crypto->plain, crypto->cipher, crypto->nbyte, AUTH_PURE_CODE_IS_ENCRYPT);
    return crypto_result;
  }

  if (event == BOOTLOADER_EVENT_DECRYPT) {
    bootloader_event_crypto_t *crypto = args;
    const int crypto_result = auth_pure_code_encrypt_decrypt(
      crypto->iv, crypto->plain, crypto->cipher, crypto->nbyte, AUTH_PURE_CODE_IS_DECRYPT);
    return crypto_result;
  }
#endif

  return 0;
}
