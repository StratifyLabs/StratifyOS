// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup QEI_DEV QEI
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_QEI_H_
#define _MCU_QEI_H_

#include "sos/dev/qei.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 value;
} qei_event_data_t;

typedef struct MCU_PACK {
  u32 port;
  qei_attr_t attr;
} qei_config_t;

int mcu_qei_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_qei_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_qei_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_qei_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_qei_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_qei_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_qei_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_qei_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

int mcu_qei_get(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_qei_getvelocity(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_qei_getindex(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* _MCU_QEI_H_ */

/*! @} */
