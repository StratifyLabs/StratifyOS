// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup EINT_DEV EINT
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_EINT_H_
#define _MCU_EINT_H_

#include "sos/dev/eint.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 status;
  u32 rising;
  u32 falling;
} eint_event_t;

typedef struct MCU_PACK {
  u32 port;
  eint_attr_t attr; // default attributes
} eint_config_t;

int mcu_eint_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_eint_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_eint_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_eint_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_eint_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_eint_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_eint_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_eint_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* EINT_H_ */

/*! @} */
