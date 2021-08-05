
#include "device/auth_flash.h"
#include "sos/config.h"

#include "sos/debug.h"

#define REQUIRE_SIGNATURE CONFIG_BOOT_IS_VERIFY_SIGNATURE

static const u8 *get_public_key() {
  return (const u8 *)((u32)sos_config.sys.secret_key_address & ~0x01);
}

int auth_flash_open(const devfs_handle_t *handle) {
  DEVFS_DRIVER_DECLARE_CONFIG_STATE(auth_flash);
  state->ecc_context = NULL;
  state->sha_context = NULL;
  return config->device.driver.open(&(config->device.handle));
}

int auth_flash_read(const devfs_handle_t *handle, devfs_async_t *async) {
  DEVFS_DRIVER_DECLARE_CONFIG(auth_flash);
  return config->device.driver.read(&(config->device.handle), async);
}

int auth_flash_write(const devfs_handle_t *handle, devfs_async_t *async) {
  DEVFS_DRIVER_DECLARE_CONFIG(auth_flash);
  return config->device.driver.write(&(config->device.handle), async);
}

int auth_flash_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  DEVFS_DRIVER_DECLARE_CONFIG_STATE(auth_flash);

#if REQUIRE_SIGNATURE
  const crypt_ecc_api_t *ecc_api =
    sos_config.sys.kernel_request_api(CRYPT_ECC_ROOT_API_REQUEST);
  const crypt_hash_api_t *sha_api =
    sos_config.sys.kernel_request_api(CRYPT_SHA256_ROOT_API_REQUEST);
#endif

  switch (request) {

  case I_FLASH_IS_SIGNATURE_REQUIRED:
    // if REQUIRE_SIGNATURE is 0, siganture is not checked
    return 1;

  case I_FLASH_GETOSINFO: {
    flash_os_info_t *info = ctl;
    info->start = config->os_start;
    info->size = config->os_size;
    return 0;
  }

  case I_FLASH_WRITEPAGE: {
    flash_writepage_t *write_page = ctl;

    if (write_page->nbyte > sizeof(write_page->buf)) {
      return SYSFS_SET_RETURN(EINVAL);
    }

    if (write_page->addr < config->os_start) {
      return SYSFS_SET_RETURN(EPERM);
    }

    if (write_page->addr + write_page->nbyte > config->os_start + config->os_size) {
      return SYSFS_SET_RETURN(EPERM);
    }

    if (write_page->addr == config->os_start) {

      if (write_page->nbyte < config->os_start_size) {
        return SYSFS_SET_RETURN(EPERM);
      }

      // intercept the first page and start hashing
#if REQUIRE_SIGNATURE
      state->ecc_context = state->ecc_context_buffer;
      ecc_api->init(&state->ecc_context);
      state->sha_context = state->sha_context_buffer;
      sha_api->init(&state->sha_context);

      sha_api->start(state->sha_context);
      sha_api->update(state->sha_context, write_page->buf, write_page->nbyte);
#endif
      state->first_page = *write_page;

      // truncate the first page and write it
      state->first_page.addr = write_page->addr + config->os_start_size;
      state->first_page.nbyte = write_page->nbyte - config->os_start_size;
      memcpy(
        state->first_page.buf, write_page->buf + config->os_start_size,
        state->first_page.nbyte);

      const int write_result = config->device.driver.ioctl(
        &(config->device.handle), I_FLASH_WRITEPAGE, &state->first_page);
      if (write_result < 0) {
        return write_result;
      }

      state->first_page.addr = write_page->addr;
      state->first_page.nbyte = config->os_start_size;
      memcpy(state->first_page.buf, write_page->buf, config->os_start_size);

      // pretend it was successful
      return write_page->nbyte;
    } else {

#if REQUIRE_SIGNATURE
      if (state->ecc_context == NULL) {
        return SYSFS_SET_RETURN(EINVAL);
      }

      sha_api->update(state->sha_context, write_page->buf, write_page->nbyte);
#endif
    }

    break;
  }

  case I_FLASH_VERIFY_SIGNATURE: {
#if REQUIRE_SIGNATURE
    u8 hash[32];
    sha_api->finish(state->sha_context, hash, sizeof(hash));
    auth_signature_t *signature = ctl;

    const u8 *public_key = get_public_key();

    ecc_api->dsa_set_key_pair(
      state->ecc_context, public_key, sos_config.sys.secret_key_size, NULL, 0);

    const int is_verified = ecc_api->dsa_verify(
      state->ecc_context, hash, sizeof(hash), signature->data, sizeof(signature->data));

    ecc_api->deinit(&state->ecc_context);
    sha_api->deinit(&state->sha_context);

    state->ecc_context = NULL;
    state->sha_context = NULL;

#else
    const int is_verified = 1;
#endif

    if (is_verified) {
      // write the first page
      config->device.driver.ioctl(
        &(config->device.handle), I_FLASH_WRITEPAGE, &state->first_page);
      state->first_page = (flash_writepage_t){};
    } else {
      return SYSFS_SET_RETURN(EINVAL);
    }

    return 0;
  }
  }

  return config->device.driver.ioctl(&(config->device.handle), request, ctl);
}

int auth_flash_close(const devfs_handle_t *handle) {
  DEVFS_DRIVER_DECLARE_CONFIG(auth_flash);
  return config->device.driver.close(&(config->device.handle));
}
