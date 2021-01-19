// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_FS_DRIVE_ASSETFS_H_
#define SOS_FS_DRIVE_ASSETFS_H_

#include <sdk/types.h>

#include "../link/types.h"
#include "sos/dev/drive.h"
#include "sysfs.h"
#include "types.h"

int drive_assetfs_init(const void *cfg);
int drive_assetfs_exit(const void *cfg);
int drive_assetfs_startup(const void *cfg);
int drive_assetfs_open(
  const void *cfg,
  void **handle,
  const char *path,
  int flags,
  int mode);
int drive_assetfs_read(
  const void *cfg,
  void *handle,
  int flags,
  int loc,
  void *buf,
  int nbyte);
int drive_assetfs_ioctl(const void *cfg, void *handle, int request, void *ctl);
int drive_assetfs_close(const void *cfg, void **handle);
int drive_assetfs_fstat(const void *cfg, void *handle, struct stat *st);
int drive_assetfs_stat(const void *cfg, const char *path, struct stat *st);
int drive_assetfs_opendir(const void *cfg, void **handle, const char *path);
int drive_assetfs_readdir_r(const void *cfg, void *handle, int loc, struct dirent *entry);
int drive_assetfs_closedir(const void *cfg, void **handle);

typedef struct MCU_PACK {
  char name[ASSETFS_NAME_MAX + 1];
  u32 start;
  u32 size;
  u16 uid;
  u16 mode;
} drive_assetfs_dirent_t;

typedef struct {
  u32 count;
  const drive_assetfs_dirent_t entries[];
} drive_assetfs_header_t;

#if !defined __link
typedef struct {
  sysfs_shared_config_t drive;
  u32 offset;
} drive_assetfs_config_t;

typedef struct {
  sysfs_shared_state_t drive;
} drive_assetfs_state_t;

#define DRIVE_ASSETFS_MOUNT(mount_loc_name, cfgp, permissions_value, owner_value)        \
  {                                                                                      \
    .mount_path = mount_loc_name, .permissions = permissions_value,                      \
    .owner = owner_value, .mount = drive_assetfs_init, .unmount = drive_assetfs_exit,    \
    .ismounted = sysfs_always_mounted, .startup = drive_assetfs_startup,                 \
    .mkfs = SYSFS_NOTSUP, .open = drive_assetfs_open, .aio = SYSFS_NOTSUP,               \
    .read = drive_assetfs_read, .write = SYSFS_NOTSUP, .close = drive_assetfs_close,     \
    .ioctl = drive_assetfs_ioctl, .rename = SYSFS_NOTSUP, .fsync = SYSFS_NOTSUP,         \
    .unlink = SYSFS_NOTSUP, .mkdir = SYSFS_NOTSUP, .rmdir = SYSFS_NOTSUP,                \
    .remove = SYSFS_NOTSUP, .opendir = drive_assetfs_opendir,                            \
    .closedir = drive_assetfs_closedir, .readdir_r = drive_assetfs_readdir_r,            \
    .link = SYSFS_NOTSUP, .symlink = SYSFS_NOTSUP, .stat = drive_assetfs_stat,           \
    .lstat = SYSFS_NOTSUP, .fstat = drive_assetfs_fstat, .chmod = SYSFS_NOTSUP,          \
    .chown = SYSFS_NOTSUP, .unlock = SYSFS_NOTSUP_VOID, .config = cfgp,                  \
  }

#endif

#endif /* SOS_FS_DRIVE_ASSETFS_H_ */
