// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup USB_DEV USB
 * @{
 * \ingroup DEV
 *
 */

#ifndef _MCU_USB_H_
#define _MCU_USB_H_

#include "sos/dev/usb.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  u8 epnum;
} usb_event_t;

typedef struct MCU_PACK {
  usb_attr_t attr; // default attributes
  u32 port;
} usb_config_t;

int mcu_usb_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_usb_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_usb_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_usb_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_usb_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_usb_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_usb_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_usb_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_usb_isconnected(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

// Endpoint functions
int mcu_usb_root_read_endpoint(const devfs_handle_t *handle, u32 endpoint_num, void *dest)
  MCU_ROOT_CODE;
int mcu_usb_root_write_endpoint(
  const devfs_handle_t *handle,
  u32 endpoint_num,
  const void *src,
  u32 size) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* _MCU_USB_H_ */

/*! @} */
