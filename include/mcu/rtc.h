/* Copyright 2011-2018 Tyler Gilbert; 
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

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
} rtc_config_t;

int mcu_rtc_open(const devfs_handle_t * handle) MCU_ROOT_CODE;
int mcu_rtc_read(const devfs_handle_t * handle, devfs_async_t * rop) MCU_ROOT_CODE;
int mcu_rtc_write(const devfs_handle_t * handle, devfs_async_t * wop) MCU_ROOT_CODE;
int mcu_rtc_ioctl(const devfs_handle_t * handle, int request, void * ctl) MCU_ROOT_CODE;
int mcu_rtc_close(const devfs_handle_t * handle) MCU_ROOT_CODE;

int mcu_rtc_getinfo(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_rtc_setattr(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_rtc_setaction(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;

int mcu_rtc_set(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;
int mcu_rtc_get(const devfs_handle_t * handle, void * ctl) MCU_ROOT_CODE;

#ifdef __cplusplus
}
#endif

#endif /* _MCU_RTC_H_ */

/*! @} */
