#ifndef AUTH_FLASH_H
#define AUTH_FLASH_H

#include "sos/fs/devfs.h"
#include "sos/dev/flash.h"

typedef struct {
  u8 ecc_context_buffer[256];
  void *ecc_context;
  u8 sha_context_buffer[256];
  void *sha_context;
  flash_writepage_t first_page;
} auth_flash_state_t;

typedef struct {
  devfs_device_t device;
  u32 os_start;
  u32 os_start_size;
  u32 os_size;
} auth_flash_config_t;


int auth_flash_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int auth_flash_read(const devfs_handle_t * handle, devfs_async_t * rop) MCU_ROOT_CODE;
int auth_flash_write(const devfs_handle_t * handle, devfs_async_t * wop) MCU_ROOT_CODE;
int auth_flash_ioctl(const devfs_handle_t * handle, int request, void * ctl) MCU_ROOT_CODE;
int auth_flash_close(const devfs_handle_t * handle) MCU_ROOT_CODE;

#endif // AUTH_FLASH_H
