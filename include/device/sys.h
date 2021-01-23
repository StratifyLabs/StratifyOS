// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_SYS_H_
#define DEVICE_SYS_H_

#include "sos/dev/sys.h"
#include "sos/dev/spi.h"

#include "sos/fs/devfs.h"
#include "cortexm/task.h"

int sys_open(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;
int sys_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_EXEC_CODE;
int sys_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_EXEC_CODE;
int sys_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_EXEC_CODE;
int sys_close(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;

int sys_23_open(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;
int sys_23_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_EXEC_CODE;
int sys_23_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_EXEC_CODE;
int sys_23_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_EXEC_CODE;
int sys_23_close(const devfs_handle_t *handle) MCU_ROOT_EXEC_CODE;

#endif /* DEVICE_SYS_H_ */
