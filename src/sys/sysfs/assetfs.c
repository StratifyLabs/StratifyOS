// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "sos/fs/assetfs.h"
#include "cortexm/cortexm.h"
#include "dirent.h"
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
  const void *data;
  u32 size;
  u32 checksum;
} assetfs_handle_t;

int assetfs_init(const void *cfg) {
  // nothing to initialize
  return 0;
}

static int get_directory_entry(const void *cfg, int loc, const assetfs_dirent_t **entry);
static const assetfs_dirent_t *find_file(const void *cfg, const char *path, int *ino);
static void assign_stat(int ino, const assetfs_dirent_t *entry, struct stat *st);

int assetfs_startup(const void *cfg) {
  // check for any applications that are embedded and start them?
  return 0;
}

int assetfs_open(const void *cfg, void **handle, const char *path, int flags, int mode) {
  MCU_UNUSED_ARGUMENT(mode);

  if ((flags & O_ACCMODE) != O_RDONLY) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  int ino;
  const assetfs_dirent_t *directory_entry = find_file(cfg, path, &ino);

  if (sysfs_is_r_ok(directory_entry->mode, directory_entry->uid, SYSFS_GROUP) == 0) {
    return SYSFS_SET_RETURN(EPERM);
  }

  assetfs_handle_t *h = malloc(sizeof(assetfs_handle_t));
  if (h == 0) {
    return -1;
  }

  h->ino = ino;
  h->data = (const void *)(directory_entry->start);
  h->size = directory_entry->end - directory_entry->start;
  cortexm_assign_zero_sum32(h, sizeof(assetfs_handle_t) / sizeof(u32));

  *handle = h;
  return 0;
}

int assetfs_read(
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
  assetfs_handle_t *h = handle;
  if (cortexm_verify_zero_sum32(h, sizeof(assetfs_handle_t) / sizeof(u32)) == 0) {
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

  memcpy(buf, (u8 *)h->data + loc, bytes_ready);
  return bytes_ready;
}

int assetfs_ioctl(const void *cfg, void *handle, int request, void *ctl) {
  MCU_UNUSED_ARGUMENT(cfg);
  MCU_UNUSED_ARGUMENT(handle);
  MCU_UNUSED_ARGUMENT(request);
  MCU_UNUSED_ARGUMENT(ctl);
  errno = ENOTSUP;
  return -1;
}

int assetfs_close(const void *cfg, void **handle) {
  MCU_UNUSED_ARGUMENT(cfg);
  if (*handle != 0) {
    if (cortexm_verify_zero_sum32(*handle, sizeof(assetfs_handle_t) / sizeof(u32)) == 0) {
      return SYSFS_SET_RETURN(EINVAL);
    }
    free(*handle);
    *handle = 0;
  }
  return 0;
}

int assetfs_fstat(const void *cfg, void *handle, struct stat *st) {
  MCU_UNUSED_ARGUMENT(cfg);
  assetfs_handle_t *h = handle;
  const assetfs_dirent_t *directory_entry;
  const assetfs_config_t *config = cfg;
  if (cortexm_verify_zero_sum32(h, sizeof(assetfs_handle_t) / sizeof(u32)) == 0) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  directory_entry = config->entries + h->ino;
  assign_stat(h->ino, directory_entry, st);

  return 0;
}

int assetfs_stat(const void *cfg, const char *path, struct stat *st) {
  int ino;
  const assetfs_dirent_t *directory_entry = find_file(cfg, path, &ino);
  if (directory_entry == 0) {
    return SYSFS_SET_RETURN(ENOENT);
  }

  assign_stat(ino, directory_entry, st);
  return 0;
}

void assign_stat(int ino, const assetfs_dirent_t *entry, struct stat *st) {
  memset(st, 0, sizeof(struct stat));
  st->st_size = entry->end - entry->start;
  st->st_ino = ino;
  st->st_mode = entry->mode | S_IFREG;
  st->st_uid = entry->uid;
}

int assetfs_opendir(const void *cfg, void **handle, const char *path) {
  if (strncmp(path, "", PATH_MAX) == 0) {
    *handle = VALID_DIR_HANDLE;
  } else {
    return SYSFS_SET_RETURN(ENOENT);
  }
  return 0;
}

int assetfs_readdir_r(const void *cfg, void *handle, int loc, struct dirent *entry) {
  if (handle != VALID_DIR_HANDLE) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  const assetfs_dirent_t *directory_entry = 0;
  int result = get_directory_entry(cfg, loc, &directory_entry);
  if (result < 0) {
    return result;
  }

  entry->d_name[NAME_MAX - 1] = 0;
  strncpy(entry->d_name, directory_entry->name, NAME_MAX - 1);
  entry->d_ino = loc;

  return 0;
}

int assetfs_closedir(const void *cfg, void **handle) {
  if (*handle != VALID_DIR_HANDLE) {
    return SYSFS_SET_RETURN(EINVAL);
  }
  *handle = INVALID_DIR_HANDLE;
  return 0;
}

const assetfs_dirent_t *find_file(const void *cfg, const char *path, int *ino) {
  int loc = 0;
  const assetfs_dirent_t *directory_entry = 0;

  while (get_directory_entry(cfg, loc, &directory_entry) == 0) {
    if (strncmp(path, directory_entry->name, NAME_MAX - 1) == 0) {
      *ino = loc;
      return directory_entry;
    }
    loc++;
  }

  return 0;
}

int get_directory_entry(const void *cfg, int loc, const assetfs_dirent_t **entry) {
  const assetfs_config_t *config = cfg;
  u32 count = config->count;
  if (loc < 0) {
    return SYSFS_SET_RETURN(EINVAL);
  }
  if (loc >= count) {
    return -1;
  }
  *entry = config->entries + loc;
  return 0;
}
