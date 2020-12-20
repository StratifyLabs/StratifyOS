// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup PIO_DEV PIO
 *
 * \ingroup DEV
 *
 */

#ifndef _MCU_PIO_H_
#define _MCU_PIO_H_

#include "core.h"
#include "sos/dev/pio.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 status;
  u32 rising;
  u32 falling;
} pio_event_data_t;

typedef struct {
  u32 port;
} pio_config_t;

int mcu_pio_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_pio_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_pio_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_pio_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_pio_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_pio_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pio_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pio_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

int mcu_pio_setmask(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pio_clrmask(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pio_get(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pio_set(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* _MCU_PIO_H_ */

/*! @} */
