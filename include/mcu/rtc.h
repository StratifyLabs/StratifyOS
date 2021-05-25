// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup RTC_DEV RTC
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_RTC_H_
#define _MCU_RTC_H_

#include "sos/dev/rtc.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 value;
} rtc_event_data_t;

typedef struct {
  rtc_time_t time;
} rtc_event_t;

typedef struct {
  rtc_attr_t attr;
  u32 port;
} rtc_config_t;

int mcu_rtc_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_rtc_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_rtc_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_rtc_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_rtc_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_rtc_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_rtc_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_rtc_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

int mcu_rtc_set(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_rtc_get(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* _MCU_RTC_H_ */

/*! @} */
