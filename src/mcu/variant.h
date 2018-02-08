/* Copyright 2011-2016 Tyler Gilbert;
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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef MCU_VARIANT_H_
#define MCU_VARIANT_H_

#include "mcu/mcu.h"
#include "local.h"


#define MCU_DRIVER_DECLARATION_POWER_ON(driver_name) extern void mcu_##driver_name##_dev_power_on(const devfs_handle_t *) MCU_ROOT_CODE
#define MCU_DRIVER_DECLARATION_POWER_OFF(driver_name) extern void mcu_##driver_name##_dev_power_off(const devfs_handle_t *) MCU_ROOT_CODE
#define MCU_DRIVER_DECLARATION_IS_POWERED(driver_name) extern int mcu_##driver_name##_dev_is_powered(const devfs_handle_t *) MCU_ROOT_CODE
#define MCU_DRIVER_DECLARATION_READ(driver_name) extern int mcu_##driver_name##_dev_read(const devfs_handle_t *, devfs_async_t *) MCU_ROOT_CODE
#define MCU_DRIVER_DECLARATION_WRITE(driver_name) extern int mcu_##driver_name##_dev_write(const devfs_handle_t *, devfs_async_t *) MCU_ROOT_CODE
#define MCU_DRIVER_DECLARATION_GET_VERSION(driver_name, version) static int mcu_##driver_name##_get_version(const devfs_handle_t * handle, void* ctl){ return version; }

#define MCU_DRIVER_DECLARATION(driver_name, version) \
    MCU_DRIVER_DECLARATION_POWER_ON(driver_name); \
    MCU_DRIVER_DECLARATION_POWER_OFF(driver_name); \
    MCU_DRIVER_DECLARATION_IS_POWERED(driver_name); \
    MCU_DRIVER_DECLARATION_READ(driver_name); \
    MCU_DRIVER_DECLARATION_WRITE(driver_name); \
    MCU_DRIVER_DECLARATION_GET_VERSION(driver_name, version)

#define MCU_DRIVER_DECLARATION_LOCAL_OPEN(driver_name) \
    int mcu_##driver_name##_open(const devfs_handle_t * handle){ return mcu_open(handle, mcu_##driver_name##_dev_is_powered, mcu_##driver_name##_dev_power_on); }

#define MCU_DRIVER_DECLARATION_LOCAL_IOCTL(driver_name, ioctl_total) int mcu_##driver_name##_ioctl(const devfs_handle_t * handle, int request, void * ctl){ \
    return mcu_ioctl(handle, \
        request, \
        ctl, \
        mcu_##driver_name##_dev_is_powered, \
        mcu_##driver_name##_ioctl_func_table, \
        ioctl_total); }

#define MCU_DRIVER_DECLARATION_LOCAL_READ(driver_name) int mcu_##driver_name##_read(const devfs_handle_t * handle, devfs_async_t * rop){ \
    return mcu_read(handle, \
        rop, \
        mcu_##driver_name##_dev_is_powered, \
        mcu_##driver_name##_dev_read); }

#define MCU_DRIVER_DECLARATION_LOCAL_WRITE(driver_name) int mcu_##driver_name##_write(const devfs_handle_t * handle, devfs_async_t * rop){ \
    return mcu_read(handle, \
        rop, \
        mcu_##driver_name##_dev_is_powered, \
        mcu_##driver_name##_dev_write); }

#define MCU_DRIVER_DECLARATION_LOCAL_NO_WRITE(driver_name) int mcu_##driver_name##_write(const devfs_handle_t * handle, devfs_async_t * rop){ \
    errno = ENOTSUP; return -1; }

#define MCU_DRIVER_DECLARATION_LOCAL_NO_READ(driver_name) int mcu_##driver_name##_write(const devfs_handle_t * handle, devfs_async_t * rop){ \
    errno = ENOTSUP; return -1; }

#define MCU_DRIVER_DECLARATION_LOCAL_CLOSE(driver_name) int mcu_##driver_name##_close(const devfs_handle_t * handle){ \
    return mcu_close(handle, mcu_##driver_name##_dev_is_powered, mcu_##driver_name##_dev_power_off); }

#define MCU_DRIVER_DECLARATION_LOCAL(driver_name, ioctl_total) \
    MCU_DRIVER_DECLARATION_LOCAL_OPEN(driver_name) \
    MCU_DRIVER_DECLARATION_LOCAL_CLOSE(driver_name) \
    MCU_DRIVER_DECLARATION_LOCAL_IOCTL(driver_name, ioctl_total) \
    MCU_DRIVER_DECLARATION_LOCAL_READ(driver_name) \
    MCU_DRIVER_DECLARATION_LOCAL_WRITE(driver_name)


#endif




