// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "sos/debug.h"
#include "sos/fs/sysfs.h"
#include <fcntl.h>
#include <unistd.h>

extern int
devfs_open(const void *cfg, void **handle, const char *path, int flags, int mode);
static void update_loc(sysfs_file_t *file, int adjust);

int sysfs_file_open(sysfs_file_t *file, const char *name, int mode) {
  int ret;
  struct stat st;
  const sysfs_t *fs = file->fs;

  // check fs level permissions
  if (
    sysfs_access(
      fs->permissions, fs->owner, SYSFS_GROUP,
      X_OK // is caller allowed to enter the directory
      )
    < 0) {
    // SYSFS_SET_RETURN(EPERM)
    errno = EPERM;
    return -1 * __LINE__;
  }

  ret = fs->open(fs->config, &(file->handle), name, file->flags, mode);
  if (ret >= 0) {
    if (fs->open == devfs_open) {
      if ((ret = fs->fstat(fs->config, file->handle, &st)) == 0) {
        if ((st.st_mode & S_IFMT) == S_IFCHR) {
          file->flags |= O_CHAR;
        }
      }
    }
  }
  SYSFS_PROCESS_RETURN(ret);
  return ret;
}

int sysfs_file_read(sysfs_file_t *file, void *buf, int nbyte) {
  const sysfs_t *fs = file->fs;
  int bytes;
  bytes = fs->read(fs->config, file->handle, file->flags, file->loc, buf, nbyte);
  SYSFS_PROCESS_RETURN(bytes);
  update_loc(file, bytes);
  return bytes;
}

int sysfs_file_write(sysfs_file_t *file, const void *buf, int nbyte) {
  const sysfs_t *fs = file->fs;
  int bytes;
  bytes = fs->write(fs->config, file->handle, file->flags, file->loc, buf, nbyte);
  SYSFS_PROCESS_RETURN(bytes);
  update_loc(file, bytes);
  return bytes;
}

int sysfs_file_aio(sysfs_file_t *file, void *aiocbp) {
  const sysfs_t *fs = file->fs;
  int ret = fs->aio(fs->config, file->handle, aiocbp);
  SYSFS_PROCESS_RETURN(ret);
  return ret;
}

int sysfs_file_close(sysfs_file_t *file) {
  const sysfs_t *fs = file->fs;
  int ret = fs->close(fs->config, &file->handle);
  SYSFS_PROCESS_RETURN(ret);
  return ret;
}

int sysfs_file_ioctl(sysfs_file_t *file, int request, void *ctl) {
  const sysfs_t *fs = file->fs;
  int ret = fs->ioctl(fs->config, file->handle, request, ctl);
  SYSFS_PROCESS_RETURN(ret);
  return ret;
}

int sysfs_file_fsync(sysfs_file_t *file) {
  const sysfs_t *fs = file->fs;
  int ret = fs->fsync(fs->config, file->handle);
  SYSFS_PROCESS_RETURN(ret);
  return ret;
}

void update_loc(sysfs_file_t *file, int adjust) {
  // if not a char device, increment the location
  if ((adjust > 0) && ((file->flags & O_CHAR) == 0)) {
    file->loc += adjust;
  }
}

int sysfs_shared_open(const sysfs_shared_config_t *config) {
  config->state->file.fs = config->devfs;
  config->state->file.flags = O_RDWR;
  config->state->file.loc = 0;
  config->state->file.handle = NULL;
  return sysfs_file_open(&(config->state->file), config->name, O_RDWR);
}

int sysfs_shared_ioctl(const sysfs_shared_config_t *config, int request, void *ctl) {
  if (config->state->file.fs == 0) {
    return SYSFS_SET_RETURN(ENODEV);
  }
  return sysfs_file_ioctl(&(config->state->file), request, ctl);
}

int sysfs_shared_fsync(const sysfs_shared_config_t *config) { return 0; }

int sysfs_shared_read(
  const sysfs_shared_config_t *config,
  int loc,
  void *buf,
  int nbyte) {
  if (config->state->file.fs == NULL) {
    return SYSFS_SET_RETURN(ENODEV);
  }
  config->state->file.loc = loc;
  return sysfs_file_read(&(config->state->file), buf, nbyte);
}

int sysfs_shared_lock(const sysfs_shared_config_t *config) {
  return pthread_mutex_lock(&config->state->mutex);
}

int sysfs_shared_unlock(const sysfs_shared_config_t *config) {
  return pthread_mutex_unlock(&config->state->mutex);
}

int sysfs_shared_write(
  const sysfs_shared_config_t *config,
  int loc,
  const void *buf,
  int nbyte) {
  if (config->state->file.fs == 0) {
    return SYSFS_SET_RETURN(ENODEV);
  }
  config->state->file.loc = loc;
  return sysfs_file_write(&(config->state->file), buf, nbyte);
}

int sysfs_shared_aio(const sysfs_shared_config_t *config, void *aio) {
  if (config->state->file.fs == 0) {
    return SYSFS_SET_RETURN(ENODEV);
  }
  return sysfs_file_aio(&(config->state->file), aio);
}

int sysfs_shared_close(const sysfs_shared_config_t *config) {
  if (config->state->file.fs == 0) {
    return SYSFS_SET_RETURN(ENODEV);
  }
  return sysfs_file_close(&(config->state->file));
}
