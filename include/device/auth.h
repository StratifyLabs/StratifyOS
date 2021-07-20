// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_AUTH_H_
#define DEVICE_AUTH_H_

#include <sdk/api.h>

#include "sos/dev/auth.h"
#include "sos/fs/types.h"

int auth_open(const devfs_handle_t *handle);
int auth_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int auth_read(const devfs_handle_t *handle, devfs_async_t *async);
int auth_write(const devfs_handle_t *handle, devfs_async_t *async);
int auth_close(const devfs_handle_t *handle);

#define AUTH_PURE_CODE_KEY_IS_FIRST 1
#define AUTH_PURE_CODE_KEY_IS_SECOND 0

int auth_pure_code_calculate_authentication(
  auth_token_t *dest,
  const auth_token_t *input,
  int key_is_first);

#define AUTH_PURE_CODE_IS_ENCRYPT 1
#define AUTH_PURE_CODE_IS_DECRYPT 0

int auth_pure_code_encrypt_decrypt(
  u8* iv, //iv size is always 16
  const u8 *source,
  u8 *dest,
  size_t nbyte,
  int is_encrypt);

extern const crypt_random_api_t random_root_api;

#endif /* DEVICE_AUTH_H_ */
