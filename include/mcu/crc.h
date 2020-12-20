// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef MCU_CRC_H_
#define MCU_CRC_H_

#include "sos/dev/crc.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 value;
} rng_event_data_t;

u32 mcu_calc_crc32(u32 seed, u32 polynomial, const u8 *buffer, u32 nbyte);
u16 mcu_calc_crc16(u16 seed, u16 polynomial, const u8 *buffer, u32 nbyte);
u8 mcu_calc_crc7(u8 seed, u8 polynomial, const u8 *chr, u32 len);

int mcu_crc_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_crc_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_crc_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_crc_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_crc_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_crc_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_crc_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_crc_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

int mcu_crc_get(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* MCU_CRC_H_ */
