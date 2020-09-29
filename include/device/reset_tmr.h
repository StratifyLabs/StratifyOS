#ifndef DEVICE_RESET_TMR_H
#define DEVICE_RESET_TMR_H

#include "sos/fs/devfs.h"

typedef struct {
  const devfs_device_t *tmr_device;
} reset_tmr_config_t;

typedef struct {
  u32 o_flags;
} reset_tmr_state_t;

int reset_tmr_open(const devfs_handle_t *handle);
int reset_tmr_ioctl(const devfs_handle_t *handle, int request, void *ctl);
int reset_tmr_read(const devfs_handle_t *handle, devfs_async_t *rop);
int reset_tmr_write(const devfs_handle_t *handle, devfs_async_t *wop);
int reset_tmr_close(const devfs_handle_t *handle);

#endif // DEVICE_RESET_TMR_H
