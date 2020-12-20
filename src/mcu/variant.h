// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


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
    return SYSFS_SET_RETURN(ENOTSUP); }

#define MCU_DRIVER_DECLARATION_LOCAL_NO_READ(driver_name) int mcu_##driver_name##_write(const devfs_handle_t * handle, devfs_async_t * rop){ \
    return SYSFS_SET_RETURN(ENOTSUP); }

#define MCU_DRIVER_DECLARATION_LOCAL_CLOSE(driver_name) int mcu_##driver_name##_close(const devfs_handle_t * handle){ \
    return mcu_close(handle, mcu_##driver_name##_dev_is_powered, mcu_##driver_name##_dev_power_off); }

#define MCU_DRIVER_DECLARATION_LOCAL(driver_name, ioctl_total) \
    MCU_DRIVER_DECLARATION_LOCAL_OPEN(driver_name) \
    MCU_DRIVER_DECLARATION_LOCAL_CLOSE(driver_name) \
    MCU_DRIVER_DECLARATION_LOCAL_IOCTL(driver_name, ioctl_total) \
    MCU_DRIVER_DECLARATION_LOCAL_READ(driver_name) \
    MCU_DRIVER_DECLARATION_LOCAL_WRITE(driver_name)


#endif




