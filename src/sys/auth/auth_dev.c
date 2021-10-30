// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>

#include "../scheduler/scheduler_root.h"
#include "config.h"
#include "cortexm/task.h"
#include "device/auth.h"

#include "sos/dev/bootloader.h"

static int
get_random(const devfs_handle_t *handle, auth_token_t *auth) MCU_ROOT_EXEC_CODE;

static int
authenticate(const devfs_handle_t *handle, auth_token_t *hash) MCU_ROOT_EXEC_CODE;

static int calculate(auth_token_t *dest, const auth_token_t *input, int key_is_first)
  MCU_ROOT_EXEC_CODE;

static void get_public_key(auth_public_key_t *public_key) MCU_ROOT_EXEC_CODE;

void get_public_key(auth_public_key_t *public_key) {
  const bootloader_api_t *api = cortexm_get_bootloader_api();
  *public_key = (auth_public_key_t){};
  api->event(BOOTLOADER_EVENT_GET_PUBLIC_KEY, public_key);
}

typedef struct {
  auth_token_t random_token;
} auth_state_t;

static auth_state_t m_auth_state MCU_SYS_MEM;

int auth_open(const devfs_handle_t *handle) {
  MCU_UNUSED_ARGUMENT(handle);
  return 0;
}

MCU_UNUSED int auth_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  int result;
  switch (request) {
  case I_AUTH_START:
    return get_random(handle, ctl);

  case I_AUTH_GETTOKEN:
    return SYSFS_SET_RETURN(ENOTSUP);

  case I_AUTH_GET_PUBLIC_KEY:
    get_public_key(ctl);
    return 0;

  case I_AUTH_FINISH:
    result = authenticate(handle, ctl);
    if (result == SYSFS_RETURN_SUCCESS) {
      scheduler_root_assert_authenticated(task_get_current());
    } else {
      scheduler_root_deassert_authenticated(task_get_current());
    }
    return result;
  default:
    break;
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

int get_random(const devfs_handle_t *handle, auth_token_t *auth) {
  MCU_UNUSED_ARGUMENT(handle);

  if (auth == 0) {
    task_deassert_root(task_get_current());
    return SYSFS_RETURN_SUCCESS;
  }

  const crypt_random_api_t *random_api =
    sos_config.sys.kernel_request_api(CRYPT_RANDOM_ROOT_API_REQUEST);
  if (random_api == NULL) {
    return SYSFS_SET_RETURN(ENOTSUP);
  }

  u8 context_buffer[random_api->get_context_size()];
  void *context = context_buffer;
  random_api->init(&context);
  random_api->random(context, auth->data + 16, 16);
  random_api->deinit(&context);

  m_auth_state.random_token = *auth;

  return SYSFS_RETURN_SUCCESS;
}

int calculate(auth_token_t *dest, const auth_token_t *input, int key_is_first) {

  const bootloader_api_t *bootloader_api = cortexm_get_bootloader_api();
  if (bootloader_api == NULL) {
    return SYSFS_SET_RETURN(ENOTSUP);
  }

  bootloader_event_authenication_t auth_event = {
    .auth_data = input->data,
    .result = dest->data,
    .is_key_first = key_is_first};

  bootloader_api->event(BOOTLOADER_EVENT_AUTHENTICATE, &auth_event);

  return 0;
}

int authenticate(const devfs_handle_t *handle, auth_token_t *hash) {
  MCU_UNUSED_ARGUMENT(handle);

  if (hash == NULL) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  // calculate hash = key + random token previously sent
  auth_token_t request_token = {};
  calculate(&request_token, &m_auth_state.random_token, AUTH_PURE_CODE_KEY_IS_FIRST);

  // compare hash received to the hash = key + random token calculated
  const int compare_result = memcmp(&request_token, hash, sizeof(auth_token_t));
  request_token = (auth_token_t){};
  if (compare_result != 0) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  calculate(hash, &m_auth_state.random_token, AUTH_PURE_CODE_KEY_IS_SECOND);

  return SYSFS_RETURN_SUCCESS;
}
