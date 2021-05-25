// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup DAC_DEV DAC
 * @{
 * \ingroup DEV
 */

#ifndef _MCU_DAC_H_
#define _MCU_DAC_H_

#include "sos/dev/dac.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 value;
} dac_event_t;

typedef struct MCU_PACK {
  dac_attr_t attr; // default attributes
  u32 port;
  u32 reference_mv;
} dac_config_t;

int mcu_dac_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_dac_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_dac_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_dac_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_dac_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_dac_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_dac_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_dac_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_dac_get(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_dac_set(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

// DMA version of the driver
int mcu_dac_dma_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_dac_dma_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_dac_dma_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_dac_dma_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_dac_dma_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_dac_dma_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_dac_dma_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_dac_dma_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_dac_dma_get(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_dac_dma_set(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif // _MCU_DAC_H_

/*! @} */
