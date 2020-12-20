// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup FLASH_DEV Flash
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_FLASH_H_
#define _MCU_FLASH_H_

#include "sos/dev/flash.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

int mcu_flash_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_flash_read(const devfs_handle_t * handle, devfs_async_t * rop) MCU_ROOT_CODE;
int mcu_flash_write(const devfs_handle_t * handle, devfs_async_t * wop) MCU_ROOT_CODE;
int mcu_flash_ioctl(const devfs_handle_t * handle, int request, void * ctl) MCU_ROOT_CODE;
int mcu_flash_close(const devfs_handle_t * handle) MCU_ROOT_CODE;

int mcu_flash_getinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_flash_setattr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_flash_setaction(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;

int mcu_flash_eraseaddr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_flash_erasepage(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_flash_getpage(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_flash_getsize(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_flash_getpageinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_flash_writepage(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif


#endif // _MCU_FLASH_H_

/*! @} */


