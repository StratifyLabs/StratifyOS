// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "sos/fs/drive_assetfs.h"
#include "cortexm/cortexm.h"
#include "dirent.h"
#include "sos/debug.h"
#include "sos/fs/sysfs.h"
#include "sos/sos.h"
#include <errno.h>
#include <sdk/types.h>
#include <string.h>
#include <sys/stat.h>

#define INVALID_DIR_HANDLE ((void *)0)
#define VALID_DIR_HANDLE ((void *)0x12345678)

typedef struct {
  int ino;
  u32 data;
  u32 size;
  u32 checksum;
} drive_assetfs_handle_t;

#define ASSETFS_CONFIG(cfg) ((drive_assetfs_config_t *)cfg)
#define ASSETFS_STATE(cfg)                                                               \
  ((drive_assetfs_state_t *)(((drive_assetfs_config_t *)cfg)->drive.state))
#define ASSETFS_DRIVE(cfg) &(((drive_assetfs_config_t *)cfg)->drive)
#define ASSETFS_DRIVE_MUTEX(cfg) &(((drive_assetfs_config_t *)cfg)->drive.state->mutex)

static int read_drive(const void *cfg, int loc, void *buf, int nbyte);
static int get_directory_entry(const void *cfg, int loc, drive_assetfs_dirent_t *entry);
static int
find_file(const void *cfg, const char *path, int *ino, drive_assetfs_dirent_t *entry);
static void assign_stat(int ino, const drive_assetfs_dirent_t *entry, struct stat *st);

int drive_assetfs_init(const void *cfg) {
  if (cfg == 0) {
    sos_debug_log_error(SOS_DEBUG_FILESYSTEM, "ASSETFS: no configuration");
    return -1;
  }

  if (ASSETFS_STATE(cfg)->drive.file.handle != 0) {
    // already initialized
    return SYSFS_RETURN_SUCCESS;
  }

  int result;
  if ((result = sysfs_shared_open(ASSETFS_DRIVE(cfg))) < 0) {
    return result;
  }

  drive_attr_t attr;
  attr.o_flags = DRIVE_FLAG_INIT;
  return sysfs_shared_ioctl(ASSETFS_DRIVE(cfg), I_DRIVE_SETATTR, &attr);
}

int drive_assetfs_exit(const void *cfg) {
  if (ASSETFS_STATE(cfg)->drive.file.handle != 0) {
    // already initialized
    return SYSFS_RETURN_SUCCESS;
  }

  int result = sysfs_shared_close(ASSETFS_DRIVE(cfg));

  ASSETFS_STATE(cfg)->drive.file.handle = NULL;

  return result;
}

int drive_assetfs_startup(const void *cfg) {
  // check for any applications that are embedded and start them?
  return 0;
}

int drive_assetfs_open(
  const void *cfg,
  void **handle,
  const char *path,
  int flags,
  int mode) {
  MCU_UNUSED_ARGUMENT(mode);

  if ((flags & O_ACCMODE) != O_RDONLY) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  int ino;
  drive_assetfs_dirent_t directory_entry;

  if (find_file(cfg, path, &ino, &directory_entry) < 0) {
    return SYSFS_SET_RETURN(ENOENT);
  }

  if (sysfs_is_r_ok(directory_entry.mode, directory_entry.uid, SYSFS_GROUP) == 0) {
    return SYSFS_SET_RETURN(EPERM);
  }

  drive_assetfs_handle_t *h = malloc(sizeof(drive_assetfs_handle_t));
  if (h == 0) {
    return SYSFS_SET_RETURN(ENOMEM);
  }

  h->ino = ino;
  h->data = directory_entry.start;
  h->size = directory_entry.size;

  cortexm_assign_zero_sum32(h, sizeof(drive_assetfs_handle_t) / sizeof(u32));

  *handle = h;
  return 0;
}
int drive_assetfs_read(
  const void *cfg,
  void *handle,
  int flags,
  int loc,
  void *buf,
  int nbyte) {
  MCU_UNUSED_ARGUMENT(cfg);
  if ((flags & O_ACCMODE) != O_RDONLY) {
    return SYSFS_SET_RETURN(EINVAL);
  }
  drive_assetfs_handle_t *h = handle;
  if (cortexm_verify_zero_sum32(h, sizeof(drive_assetfs_handle_t) / sizeof(u32)) == 0) {
    return SYSFS_SET_RETURN(EINVAL);
  }
  if (loc < 0) {
    return SYSFS_SET_RETURN(EINVAL);
  }
  int bytes_ready = h->size - loc;
  if (bytes_ready > nbyte) {
    bytes_ready = nbyte;
  }
  if (bytes_ready <= 0) {
    return 0;
  }

  // don't read past the end of the file
  return read_drive(cfg, h->data + loc, buf, bytes_ready);
}

int drive_assetfs_ioctl(const void *cfg, void *handle, int request, void *ctl) {
  MCU_UNUSED_ARGUMENT(cfg);
  MCU_UNUSED_ARGUMENT(handle);
  MCU_UNUSED_ARGUMENT(request);
  MCU_UNUSED_ARGUMENT(ctl);
  return SYSFS_SET_RETURN(ENOTSUP);
}

int drive_assetfs_close(const void *cfg, void **handle) {
  MCU_UNUSED_ARGUMENT(cfg);
  if (*handle != 0) {
    if (
      cortexm_verify_zero_sum32(*handle, sizeof(drive_assetfs_handle_t) / sizeof(u32))
      == 0) {
      return SYSFS_SET_RETURN(EINVAL);
    }
    free(*handle);
    *handle = 0;
  }
  return 0;
}

int drive_assetfs_fstat(const void *cfg, void *handle, struct stat *st) {
  MCU_UNUSED_ARGUMENT(cfg);
  drive_assetfs_handle_t *h = handle;
  if (cortexm_verify_zero_sum32(h, sizeof(drive_assetfs_handle_t) / sizeof(u32)) == 0) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  drive_assetfs_dirent_t directory_entry;
  get_directory_entry(cfg, h->ino, &directory_entry);
  assign_stat(h->ino, &directory_entry, st);
  return 0;
}

int drive_assetfs_stat(const void *cfg, const char *path, struct stat *st) {
  int ino;
  drive_assetfs_dirent_t directory_entry;
  if (find_file(cfg, path, &ino, &directory_entry) < 0) {
    return SYSFS_SET_RETURN(ENOENT);
  }

  assign_stat(ino, &directory_entry, st);
  return 0;
}

void assign_stat(int ino, const drive_assetfs_dirent_t *entry, struct stat *st) {
  memset(st, 0, sizeof(struct stat));
  st->st_size = entry->size;
  st->st_ino = ino;
  st->st_mode = entry->mode | S_IFREG;
  st->st_uid = entry->uid;
}

int drive_assetfs_opendir(const void *cfg, void **handle, const char *path) {
  if (strncmp(path, "", PATH_MAX) == 0) {
    *handle = VALID_DIR_HANDLE;
    return 0;
  }
  return SYSFS_SET_RETURN(ENOENT);
}

int drive_assetfs_readdir_r(
  const void *cfg,
  void *handle,
  int loc,
  struct dirent *entry) {
  if (handle != VALID_DIR_HANDLE) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  drive_assetfs_dirent_t directory_entry;
  int result = get_directory_entry(cfg, loc, &directory_entry);
  if (result < 0) {
    return result;
  }

  entry->d_name[NAME_MAX - 1] = 0;
  strncpy(entry->d_name, directory_entry.name, NAME_MAX - 1);
  entry->d_ino = loc;

  return 0;
}

int drive_assetfs_closedir(const void *cfg, void **handle) {
  if (*handle != VALID_DIR_HANDLE) {
    return SYSFS_SET_RETURN(EINVAL);
  }
  *handle = INVALID_DIR_HANDLE;
  return 0;
}

int find_file(
  const void *cfg,
  const char *path,
  int *ino,
  drive_assetfs_dirent_t *directory_entry) {
  int loc = 0;

  while (get_directory_entry(cfg, loc, directory_entry) == 0) {
    if (strncmp(path, directory_entry->name, NAME_MAX - 1) == 0) {
      *ino = loc;
      return 0;
    }
    loc++;
  }

  return -1;
}

int get_directory_entry(const void *cfg, int loc, drive_assetfs_dirent_t *entry) {
  u32 count;
  read_drive(cfg, 0, &count, sizeof(u32));
  if (count == 0xffffffff) {
    count = 0;
  }
  if (loc < 0) {
    return SYSFS_SET_RETURN(EINVAL);
  }
  if (loc >= count) {
    // end of directory -- don't set errno
    return -1;
  }
  // count plus number of entries in
  read_drive(
    cfg, loc * sizeof(drive_assetfs_dirent_t) + sizeof(u32), entry, sizeof(*entry));
  return 0;
}

int read_drive(const void *cfg, int loc, void *buf, int nbyte) {
  return sysfs_shared_read(
    ASSETFS_DRIVE(cfg), ASSETFS_CONFIG(cfg)->offset + loc, buf, nbyte);
}
