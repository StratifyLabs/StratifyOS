// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef DEVICE_DRIVE_RAM_H_
#define DEVICE_DRIVE_RAM_H_

#include "sos/dev/drive.h"
#include "sos/fs/devfs.h"

enum { DRIVE_RAM_CONFIG_FLAG_IS_READ_ONLY = (1 << 0) };

typedef struct {
  void *memory;
  u32 size;
  u32 o_flags;
} drive_ram_config_t;

DEVFS_DRIVER_DECLARTION(drive_ram);

#endif /* DEVICE_DRIVE_RAM_H_ */
