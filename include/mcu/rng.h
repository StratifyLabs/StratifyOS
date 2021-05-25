// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup rng_DEV RTC
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_rng_H_
#define _MCU_rng_H_

#include "sos/dev/random.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 value;
} rng_event_data_t;

int mcu_rng_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_rng_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_rng_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_rng_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_rng_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_rng_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_rng_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_rng_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

int mcu_rng_get(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* _MCU_rng_H_ */

/*! @} */
