// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "mcu/usb.h"
#include "sos/dev/tty.h"

int tty_usbbulk_open(const devfs_handle_t *handle) { return 0; }

int tty_usbbulk_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  return mcu_usb_ioctl(handle, request, ctl);
}

int tty_usbbulk_write(const devfs_handle_t *handle, devfs_async_t *wop) {
  const tty_config_t *dcfg;
  dcfg = (const tty_config_t *)handle->config;
  wop->loc = dcfg->write_ep;
  return mcu_usb_write(handle, wop);
}

int tty_usbbulk_read(
  const devfs_handle_t *handle,
  devfs_async_t *rop) { // The stdio is read by the USB connection
  const tty_config_t *dcfg;
  dcfg = (const tty_config_t *)handle->config;
  rop->loc = dcfg->read_ep;
  return mcu_usb_read(handle, rop);
}

int tty_usbbulk_close(const devfs_handle_t *handle) { return 0; }
