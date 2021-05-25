// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup LTDC_DEV LTDC
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_LTDC_H_
#define _MCU_LTDC_H_

#include "sos/dev/ltdc.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 status;
  u32 rising;
  u32 falling;
} ltdc_event_t;

typedef struct MCU_PACK {
  ltdc_attr_t attr; // default attributes
  u32 port;
} ltdc_config_t;

int mcu_ltdc_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_ltdc_read(const devfs_handle_t *handle, devfs_async_t *async) MCU_ROOT_CODE;
int mcu_ltdc_write(const devfs_handle_t *handle, devfs_async_t *async) MCU_ROOT_CODE;
int mcu_ltdc_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_ltdc_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_ltdc_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_ltdc_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_ltdc_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* _MCU_LTDC_H_ */

/*! @} */
