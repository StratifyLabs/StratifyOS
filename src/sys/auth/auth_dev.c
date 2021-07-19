// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include <errno.h>

#include "../scheduler/scheduler_timing.h"
#include "../scheduler/scheduler_root.h"
#include "config.h"
#include "cortexm/task.h"
#include "device/auth.h"

static const void *secret_key();
static u8 generate_pseudo_random() MCU_ROOT_EXEC_CODE;
static int
get_random(const devfs_handle_t *handle, auth_token_t *auth) MCU_ROOT_EXEC_CODE;
static int
authenticate(const devfs_handle_t *handle, auth_token_t *auth) MCU_ROOT_EXEC_CODE;
static int
calculate(auth_token_t *dest, const auth_token_t *input0, const auth_token_t *input1)
  MCU_ROOT_EXEC_CODE;

typedef struct {
  //struct tc_sha256_state_struct sha256;
  auth_token_t random_token;
} auth_state_t;

static auth_state_t m_auth_state MCU_SYS_MEM;

int auth_open(const devfs_handle_t *handle) {
  MCU_UNUSED_ARGUMENT(handle);
  return 0;
}

int auth_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  auth_key_token_t *key_token = ctl;
  int result;
  switch (request) {
  case I_AUTH_START:
    return get_random(handle, ctl);

  case I_AUTH_GETTOKEN:
    // user provides the key and the random token -- auth token is calculated
    result = calculate(&key_token->token, &key_token->key, &key_token->token);
    if (result < 0) {
      memset(key_token, 0, sizeof(auth_key_token_t));
      return SYSFS_SET_RETURN(EIO);
    }

    return 0;

  case I_AUTH_FINISH:
    result = authenticate(handle, ctl);
    if (result == SYSFS_RETURN_SUCCESS) {
      scheduler_root_assert_authenticated(task_get_current());
    } else {
      scheduler_root_deassert_authenticated(task_get_current());
    }
    return result;
  }
  return SYSFS_SET_RETURN(EINVAL);
}

int auth_read(const devfs_handle_t *handle, devfs_async_t *async) {
  MCU_UNUSED_ARGUMENT(handle);
  MCU_UNUSED_ARGUMENT(async);
  return SYSFS_SET_RETURN(ENOTSUP);
}

int auth_write(const devfs_handle_t *handle, devfs_async_t *async) {
  MCU_UNUSED_ARGUMENT(handle);
  MCU_UNUSED_ARGUMENT(async);
  return SYSFS_SET_RETURN(ENOTSUP);
}

int auth_close(const devfs_handle_t *handle) {
  MCU_UNUSED_ARGUMENT(handle);
  return 0;
}

u8 generate_pseudo_random() {
  struct mcu_timeval tval;
  u8 result;
  for (u32 i = 0; i < 32; i++) {
    scheduler_timing_root_get_realtime(&tval);
    result = tval.tv_usec;
    cortexm_delay_us(result & 0x0fff);
  }
  scheduler_timing_root_get_realtime(&tval);
  return tval.tv_usec;
}

int get_random(const devfs_handle_t *handle, auth_token_t *auth) {
  MCU_UNUSED_ARGUMENT(handle);

  if (auth == 0) {
    task_deassert_root(task_get_current());
    return SYSFS_RETURN_SUCCESS;
  }

  // generate a new random number in last 128 bits of auth
  for (u32 i = 16; i < 32; i++) {
    auth->data[i] = generate_pseudo_random();
  }

  memcpy(&m_auth_state.random_token, auth, sizeof(auth_token_t));

  return SYSFS_RETURN_SUCCESS;
}

const void *secret_key() {
  return (const void *)((u32)(sos_config.sys.secret_key_address) & ~0x01);
}

int calculate(
  auth_token_t *dest,
  const auth_token_t *input0,
  const auth_token_t *input1) {

  //if (tc_sha256_init(&m_auth_state.sha256) == TC_CRYPTO_FAIL) {
  //  return SYSFS_SET_RETURN(EIO);
  //}
  //tc_sha256_update(&m_auth_state.sha256, (const u8 *)input0, sizeof(auth_token_t));
  //tc_sha256_update(&m_auth_state.sha256, (const u8 *)input1, sizeof(auth_token_t));
  //tc_sha256_final((u8 *)dest, &m_auth_state.sha256);

  return 0;
}

int authenticate(const devfs_handle_t *handle, auth_token_t *auth) {
  MCU_UNUSED_ARGUMENT(handle);
  auth_token_t output;
  int result;

  if (secret_key() == NULL) {
    return SYSFS_RETURN_SUCCESS;
  }

  if (auth == NULL) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  if (calculate(&output, secret_key(), &m_auth_state.random_token) < 0) {
    return SYSFS_SET_RETURN(EIO);
  }

  result = memcmp(&output, auth, sizeof(auth_token_t));
  output = (auth_token_t){};
  if (result != 0) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  // allow the caller to authenticate the auth dev has the secret key
  if (calculate(auth, &m_auth_state.random_token, secret_key()) < 0) {
    *auth = (auth_token_t){};
    return SYSFS_SET_RETURN(EIO);
  }

  return SYSFS_RETURN_SUCCESS;
}
