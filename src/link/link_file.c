// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <stdarg.h>
#include <string.h>

#include "link_local.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined __win32
#define posix_open _open
#define posix_close _close
#define posix_read _read
#define posix_write _write
#define posix_lseek _lseek
#define posix_stat _stat
#define posix_fstat _fstat
#define POSIX_OPEN_FLAGS (O_BINARY)
#define posix_nbyte_t unsigned int
#else
#define posix_open open
#define posix_close close
#define posix_read read
#define posix_write write
#define posix_lseek lseek
#define posix_stat stat
#define posix_fstat fstat
#define POSIX_OPEN_FLAGS (0)
#define posix_nbyte_t int
#endif

static void convert_stat(struct stat *dest, const struct link_stat *source) {
  // dest->st_blksize = source->st_blksize;
  // dest->st_blocks = source->st_blocks;
  // dest->st_ctime_ = source->st_ctime_;
  dest->st_dev = source->st_dev;
  dest->st_ino = source->st_ino;
  dest->st_mode = source->st_mode;
  dest->st_uid = source->st_uid;
  dest->st_gid = source->st_gid;
  dest->st_rdev = source->st_rdev;
  dest->st_size = source->st_size;
  dest->st_mtime = source->st_mtime_;
  dest->st_ctime = source->st_ctime_;
}

static int convert_flags(int link_flags) {
  int result = 0;
  if (link_flags & O_CREAT) {
    result |= LINK_O_CREAT;
  }
  if (link_flags & O_APPEND) {
    result |= LINK_O_APPEND;
  }
  if (link_flags & O_EXCL) {
    result |= LINK_O_EXCL;
  }
  if (link_flags & O_RDWR) {
    result |= LINK_O_RDWR;
  }
#if defined O_NONBLOCK
  if (link_flags & O_NONBLOCK) {
    result |= LINK_O_NONBLOCK;
  }
#endif
  if (link_flags & O_WRONLY) {
    result |= LINK_O_WRONLY;
  }
  if (link_flags & O_TRUNC) {
    result |= LINK_O_TRUNC;
  }
  return result;
}

int link_open(link_transport_mdriver_t *driver, const char *path, int flags, ...) {
  link_op_t op;
  link_reply_t reply;
  link_mode_t mode;
  int err;
  va_list ap;

  if (flags & O_CREAT) {
    va_start(ap, flags);
    mode = va_arg(ap, link_mode_t);
    va_end(ap);
  } else {
    mode = 0;
  }

  if (driver == NULL) {
    link_debug(LINK_DEBUG_INFO, "posix call with (%s, 0x%X, %o)", path, flags, mode);

    int result = posix_open(path, flags | POSIX_OPEN_FLAGS, mode);

    link_debug(LINK_DEBUG_MESSAGE, "opened with file number: %d", result);

    link_errno = errno;
    return result;
  }

  link_debug(
    LINK_DEBUG_INFO, "convert flags 0x%X -> 0x%X", flags,
    convert_flags(flags) | POSIX_OPEN_FLAGS);

  link_debug(
    LINK_DEBUG_INFO, "call with (%s, 0x%X, %o) and handle %p", path, mode, flags,
    driver->phy_driver.handle);

  op.open.cmd = LINK_CMD_OPEN;
  op.open.path_size = strnlen(path, LINK_PATH_MAX) + 1;
  op.open.flags = (u32)convert_flags(flags) | LINK_O_NONBLOCK;
  op.open.mode = mode;

  if (op.open.path_size > driver->path_max) {
    link_error("name too long %d > %d", op.open.path_size, driver->path_max);
    errno = ENAMETOOLONG;
    return -1;
  }

  link_debug(LINK_DEBUG_MESSAGE, "Write open op (%p)", driver->phy_driver.handle);
  err = link_transport_masterwrite(driver, &op, sizeof(link_open_t));

  if (err < 0) {
    link_error("failed to write open op with handle %p", driver->phy_driver.handle);
    return link_handle_err(driver, err);
  }

  // Send the path on the bulk out endpoint
  link_debug(LINK_DEBUG_MESSAGE, "Write open path (%d bytes)", op.open.path_size);
  err = link_transport_masterwrite(driver, path, (int)op.open.path_size);
  if (err < 0) {
    link_error("failed to write path");
    return link_handle_err(driver, err);
  }

  //give extra time in case opening takes awhile
  link_transport_mastersettimeout(driver, 5000);

  // read the reply to see if the file opened correctly
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_error("failed to read the reply");
    return link_handle_err(driver, err);
  }

  link_transport_mastersettimeout(driver, 0);

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(
      LINK_DEBUG_WARNING, "Failed to open file (%d, %d)", reply.err, reply.err_number);
  } else {
    link_debug(LINK_DEBUG_MESSAGE, "Opened fildes: %d", reply.err);
  }
  return reply.err;
}

int link_ioctl(link_transport_mdriver_t *driver, int fildes, int request, ...) {
  void *argp;
  int arg;
  va_list ap;
  if (_IOCTL_IOCTLRW(request)) {
    va_start(ap, request);
    argp = va_arg(ap, void *);
    va_end(ap);
    arg = 0;
  } else {
    va_start(ap, request);
    arg = va_arg(ap, int);
    va_end(ap);
    argp = NULL;
  }

  return link_ioctl_delay(driver, fildes, request, argp, arg, 0);
}

int link_ioctl_delay(
  link_transport_mdriver_t *driver,
  int fildes,
  int request,
  void *argp,
  int arg,
  int delay) {
  int rw_size;
  link_op_t op;
  link_reply_t reply;

  int err;

  if (driver == 0) {
    return -1;
  }

  link_debug(
    LINK_DEBUG_INFO, "call with (%d, %d, %p/%d) and handle %p and delay %d", fildes,
    request, argp, arg, driver->phy_driver.handle, delay);

  if (fildes == LINK_BOOTLOADER_FILDES) {
    link_debug(LINK_DEBUG_INFO, "call is using the designated bootloader fildes");
  }

  rw_size = _IOCTL_SIZE(request);

  link_debug(
    LINK_DEBUG_MESSAGE, "Sending IOCTL request %c %d r:%X w:%X", _IOCTL_IDENT(request),
    _IOCTL_SIZE(request), _IOCTL_IOCTLR(request) != 0, _IOCTL_IOCTLW(request) != 0);

  // execute the request
  op.ioctl.fildes = fildes;
  op.ioctl.cmd = LINK_CMD_IOCTL;
  op.ioctl.request = request;
  op.ioctl.arg = (u32)arg;
  err = link_transport_masterwrite(driver, &op, sizeof(link_ioctl_t));
  if (err < 0) {
    link_error("failed to write op");
    return link_handle_err(driver, err);
  }

  if (_IOCTL_IOCTLW(request)) {
    // need to write data to the bulk endpoint (argp)
    link_debug(LINK_DEBUG_MESSAGE, "Sending IOW data");
    err = link_transport_masterwrite(driver, argp, rw_size);
    if (err < 0) {
      link_error("failed to write IOW data");
      return link_handle_err(driver, err);
    }
  }

  if (delay > 0) {
    // some IOCTL operations (like erasing flash, can take awhile)
    // we don't want to bother the MCU during these times
    link_debug(LINK_DEBUG_MESSAGE, "Delay for %dms", delay);
    driver->phy_driver.wait(delay);
    driver->phy_driver.wait(delay);
    driver->phy_driver.wait(delay);
  }

  if (_IOCTL_IOCTLR(request)) {
    // need to read data from the bulk endpoint
    link_debug(LINK_DEBUG_MESSAGE, "Getting IOR data %d bytes", rw_size);
    err = link_transport_masterread(driver, argp, rw_size);
    link_debug(LINK_DEBUG_MESSAGE, "Getting IOR data done (%d)", err);

    if (err < 0) {
      link_error("failed to read IOR data");
      return link_handle_err(driver, err);
    }

    if (err != rw_size) {
      if (err == sizeof(reply)) {
        link_errno = reply.err_number;
        link_error("failed to read IOR data -- bad size %d", link_errno);
        memcpy(&reply, argp, sizeof(reply));
        return reply.err;
      }

      return link_handle_err(driver, LINK_PROT_ERROR);
    }
  }

  // Get the reply
  link_debug(LINK_DEBUG_MESSAGE, "Read reply");
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_error("failed to read reply");
    return link_handle_err(driver, err);
  }

  link_errno = reply.err_number;
  link_debug(LINK_DEBUG_MESSAGE, "Replied with %d (%d)", reply.err, link_errno);
  if (reply.err < 0) {
    errno = link_errno;
    link_debug(LINK_DEBUG_WARNING, "Failed to ioctl file (%d)", link_errno);
  }
  return reply.err;
}

int link_read(link_transport_mdriver_t *driver, int fildes, void *buf, int nbyte) {
  link_op_t op;
  link_reply_t reply;
  int err;

  if (driver == 0) {
    int result = posix_read(fildes, buf, (posix_nbyte_t)nbyte);
    link_errno = errno;
    return result;
  }

  op.read.cmd = LINK_CMD_READ;
  op.read.fildes = fildes;
  op.read.nbyte = (u32)nbyte;

  link_debug(
    LINK_DEBUG_INFO, "call with (%d, %p, %d) and handle %p", fildes, buf, nbyte,
    driver->phy_driver.handle);

  link_debug(LINK_DEBUG_MESSAGE, "write read op");
  err = link_transport_masterwrite(driver, &op, sizeof(link_read_t));
  if (err < 0) {
    link_error("failed to write op");
    return link_handle_err(driver, err);
  }

  link_debug(LINK_DEBUG_MESSAGE, "read data from the file %d", nbyte);
  err = link_transport_masterread(driver, buf, nbyte);
  if (err < 0) {
    link_error("failed to read data");
    return link_handle_err(driver, err);
  }

  link_debug(LINK_DEBUG_MESSAGE, "read %d of %d bytes", err, nbyte);
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_error("failed to read reply");
    return link_handle_err(driver, err);
  }

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to read file (%d)", link_errno);
  }

  return reply.err;
}

int link_write(link_transport_mdriver_t *driver, int fildes, const void *buf, int nbyte) {

  link_op_t op;
  link_reply_t reply;
  int err;

  if (driver == NULL) {
    int result = posix_write(fildes, buf, (posix_nbyte_t)nbyte);
    if (result < 0) {
      link_error("failed to write posix file with errno %d", errno);
    }
    link_errno = errno;
    return result;
  }

  op.write.cmd = LINK_CMD_WRITE;
  op.write.fildes = fildes;
  op.write.nbyte = (u32)nbyte;

  link_debug(
    LINK_DEBUG_INFO, "call with (%d, %p, %d) and handle %p", fildes, buf, nbyte,
    driver->phy_driver.handle);

  err = link_transport_masterwrite(driver, &op, sizeof(link_write_t));
  if (err < 0) {
    link_error("failed to write op");
    return link_handle_err(driver, err);
  }

  link_debug(LINK_DEBUG_MESSAGE, "Write data");
  err = link_transport_masterwrite(driver, buf, nbyte);
  if (err < 0) {
    link_error("failed to write data");
    return link_handle_err(driver, err);
  }

  // read the reply to see if the file wrote correctly
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_error("failed to read reply");
    return link_handle_err(driver, err);
  }

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to write file (%d)", link_errno);
  }
  return reply.err;
}

int link_close(link_transport_mdriver_t *driver, int fildes) {
  if (driver == NULL) {
    link_debug(LINK_DEBUG_DEBUG, "closing fileno:%d", fildes);
    int result = posix_close(fildes);
    link_errno = errno;
    return result;
  }

  link_op_t op;
  link_reply_t reply;
  int err;

  op.close.cmd = LINK_CMD_CLOSE;
  op.close.fildes = fildes;

  link_debug(
    LINK_DEBUG_INFO, "call with (%d) and handle %p", fildes, driver->phy_driver.handle);

  err = link_transport_masterwrite(driver, &op, sizeof(link_close_t));
  if (err < 0) {
    link_error("failed to write op");
    return link_handle_err(driver, err);
  }

  link_debug(LINK_DEBUG_MESSAGE, "Read Reply");
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_error("failed to read reply");
    return link_handle_err(driver, err);
  }

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to close file (%d)", link_errno);
  }

  link_debug(LINK_DEBUG_MESSAGE, "close complete %d", reply.err);

  return reply.err;
}

int link_symlink(
  link_transport_mdriver_t *driver,
  const char *old_path,
  const char *new_path) {
  link_op_t op;
  link_reply_t reply;
  int len;
  int err;
  if (driver == NULL) {
    return LINK_TRANSFER_ERR;
  }
  op.symlink.cmd = LINK_CMD_LINK;
  op.symlink.path_size_old = strnlen(old_path, LINK_PATH_MAX_LARGE) + 1;
  op.symlink.path_size_new = strnlen(new_path, LINK_PATH_MAX_LARGE) + 1;

  link_debug(
    LINK_DEBUG_INFO, "call with (%s, %s) and handle %p", old_path, new_path,
    driver->phy_driver.handle);

  if (
    (op.symlink.path_size_old > driver->path_max)
    || (op.symlink.path_size_old > driver->path_max)) {
    errno = ENAMETOOLONG;
    return -1;
  }

  err = link_transport_masterwrite(driver, &op, sizeof(link_symlink_t));
  if (err < 0) {
    return err;
  }

  // Send the path on the bulk out endpoint
  link_debug(LINK_DEBUG_MESSAGE, "Write open path (%d bytes)", op.symlink.path_size_old);
  len = link_transport_masterwrite(driver, old_path, op.symlink.path_size_old);
  if (len != op.symlink.path_size_old) {
    return -1;
  }

  len = link_transport_masterwrite(driver, new_path, op.symlink.path_size_new);
  if (len < 0) {
    return LINK_TRANSFER_ERR;
  }

  // read the reply to see if the file opened correctly
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    return err;
  }

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to symlink (%d)", link_errno);
  }
  return reply.err;
}

int link_unlink(link_transport_mdriver_t *driver, const char *path) {
  if (driver == NULL) {
    int result = unlink(path);
    link_errno = errno;
    return result;
  }

  link_op_t op;
  link_reply_t reply;
  int len;
  int err;

  link_debug(
    LINK_DEBUG_INFO, "call with (%s) and handle %p", path, driver->phy_driver.handle);

  if (strnlen(path, LINK_PATH_MAX_LARGE) > driver->path_max) {
    errno = ENAMETOOLONG;
    return -1;
  }

  op.unlink.cmd = LINK_CMD_UNLINK;
  op.unlink.path_size = strlen(path) + 1;

  err = link_transport_masterwrite(driver, &op, sizeof(link_unlink_t));
  if (err < 0) {
    return err;
  }

  // Send the path on the bulk out endpoint
  len = link_transport_masterwrite(driver, path, op.unlink.path_size);
  if (len < 0) {
    return LINK_TRANSFER_ERR;
  }

  // some erase operations take a long time
  link_transport_mastersettimeout(driver, 10000);

  // read the reply to see if the file deleted correctly
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    return err;
  }

  // restore the timeout to the default
  link_transport_mastersettimeout(driver, 0);

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to unlink file (%d)", link_errno);
  }
  return reply.err;
}

int link_lseek(link_transport_mdriver_t *driver, int fildes, s32 offset, int whence) {

  if (driver == 0) {
    // operate on local file
    int result = posix_lseek(fildes, offset, whence);
    link_errno = errno;
    return result;
  }

  link_op_t op;
  link_reply_t reply;
  int err;

  op.lseek.cmd = LINK_CMD_LSEEK;
  op.lseek.fildes = fildes;
  op.lseek.offset = offset;
  op.lseek.whence = whence;

  link_errno = 0;

  link_debug(
    LINK_DEBUG_INFO, "call with (%d, %d, %d) and handle %p", fildes, offset, whence,
    driver->phy_driver.handle);

  err = link_transport_masterwrite(driver, &op, sizeof(link_lseek_t));
  if (err < 0) {
    return err;
  }

  link_debug(LINK_DEBUG_MESSAGE, "wait for reply");
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    return err;
  }

  link_debug(LINK_DEBUG_MESSAGE, "device returned %d %d", reply.err, reply.err_number);
  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to lseek file (%d)", link_errno);
  }
  return reply.err;
}

int link_stat(link_transport_mdriver_t *driver, const char *path, struct stat *buf) {

  if (driver == NULL) {
    struct posix_stat tmp;
    int result;
    if ((result = posix_stat(path, &tmp)) < 0) {
      return result;
    }

    memcpy(buf, &tmp, sizeof(tmp));
    return result;
  }

  link_op_t op;
  link_reply_t reply;
  int len;
  int err;

  link_debug(
    LINK_DEBUG_INFO, "call with (%s, %p) and handle %p", path, buf,
    driver->phy_driver.handle);

  if (strnlen(path, LINK_PATH_MAX_LARGE) > driver->path_max) {
    errno = ENAMETOOLONG;
    return -1;
  }

  op.stat.cmd = LINK_CMD_STAT;
  op.stat.path_size = strlen(path) + 1;

  link_debug(
    LINK_DEBUG_MESSAGE, "send op %d path size %d", op.stat.cmd, op.stat.path_size);
  err = link_transport_masterwrite(driver, &op, sizeof(link_stat_t));
  if (err < 0) {
    return err;
  }

  // Send the path on the bulk out endpoint
  link_debug(LINK_DEBUG_MESSAGE, "write stat path");
  len = link_transport_masterwrite(driver, path, op.stat.path_size);
  if (len < 0) {
    return LINK_TRANSFER_ERR;
  }

  // Get the reply
  link_debug(LINK_DEBUG_MESSAGE, "read stat reply");
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_error("failed to read reply");
    return err;
  }

  struct link_stat output;
  link_debug(LINK_DEBUG_MESSAGE, "stat reply %d", reply.err);
  if (reply.err == 0) {
    // Read bulk in as the size of the the new data
    link_debug(LINK_DEBUG_MESSAGE, "read stat data");
    err = link_transport_masterread(driver, &output, sizeof(struct link_stat));
    if (err < 0) {
      return err;
    }
  } else {
    errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to stat file (%d)", link_errno);
  }

  convert_stat(buf, &output);

  return reply.err;
}

int link_fstat(link_transport_mdriver_t *driver, int fildes, struct stat *buf) {
  if (driver == NULL) {
    link_debug(LINK_DEBUG_INFO, "posix call with (%d, %p)", fildes, buf);
#if defined(__win32)
		struct _stat result;
		if( posix_fstat(fildes, &result) < 0 ){
			return -1;
		}
		buf->st_dev = result.st_dev;
		buf->st_ino = result.st_ino;
		buf->st_mode = result.st_mode;
		buf->st_nlink = result.st_nlink;
		buf->st_uid = result.st_uid;
		buf->st_gid = result.st_gid;
		buf->st_rdev = result.st_rdev;
		buf->st_size = result.st_size;
		buf->st_atime = result.st_atime;
		buf->st_mtime = result.st_mtime;
		buf->st_ctime = result.st_ctime;
		return 0;

#else
		return posix_fstat(fildes, buf);
#endif
  }

  link_op_t op;
  link_reply_t reply;
  int err;

  op.fstat.cmd = LINK_CMD_FSTAT;
  op.fstat.fildes = fildes;

  link_debug(
    LINK_DEBUG_INFO, "call with (%d, %p) and handle %p", fildes, buf,
    driver->phy_driver.handle);

  err = link_transport_masterwrite(driver, &op, sizeof(link_fstat_t));
  if (err < 0) {
    return err;
  }

  // Get the reply
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    return err;
  }

  struct link_stat output;
  if (reply.err == 0) {
    // Read bulk in as the size of the the new data
    err = link_transport_masterread(driver, &output, sizeof(struct link_stat));
    if (err < 0) {
      return err;
    }

  } else {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to fstat file (%d)", link_errno);
  }

  convert_stat(buf, &output);

  return reply.err;
}

int link_rename(
  link_transport_mdriver_t *driver,
  const char *old_path,
  const char *new_path) {
  link_op_t op;
  link_reply_t reply;
  int len;
  int err;
  if (driver == NULL) {
    return rename(old_path, new_path);
  }

  link_debug(
    LINK_DEBUG_INFO, "call with (%s, %s) and handle %p", old_path, new_path,
    driver->phy_driver.handle);

  op.rename.cmd = LINK_CMD_RENAME;
  op.rename.old_size = strnlen(old_path, LINK_PATH_MAX_LARGE) + 1;
  op.rename.new_size = strnlen(new_path, LINK_PATH_MAX_LARGE) + 1;

  if (
    (op.rename.old_size > driver->path_max) || (op.rename.new_size > driver->path_max)) {
    errno = ENAMETOOLONG;
    return -1;
  }

  err = link_transport_masterwrite(driver, &op, sizeof(link_rename_t));
  if (err < 0) {
    return err;
  }

  // Send the old path on the bulk out endpoint
  len = link_transport_masterwrite(driver, old_path, op.rename.old_size);
  if (len < 0) {
    return LINK_TRANSFER_ERR;
  }

  // Send the new path on the bulk out endpoint
  len = link_transport_masterwrite(driver, new_path, op.rename.new_size);
  if (len < 0) {
    return LINK_TRANSFER_ERR;
  }

  // read the reply to see if the file opened correctly
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    return err;
  }

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to rename file (%d)", link_errno);
  }
  return reply.err;
}

int link_chown(link_transport_mdriver_t *driver, const char *path, int owner, int group) {
  link_op_t op;
  link_reply_t reply;
  int len;
  int err;

  if (driver == NULL) {
    return LINK_TRANSFER_ERR;
  }

  link_debug(
    LINK_DEBUG_INFO, "call with (%s, %d, %d) and handle %p", path, owner, group,
    driver->phy_driver.handle);

  if (strnlen(path, LINK_PATH_MAX_LARGE) > driver->path_max) {
    errno = ENAMETOOLONG;
    return -1;
  }

  op.chown.cmd = LINK_CMD_CHOWN;
  op.chown.path_size = strlen(path) + 1;
  op.chown.uid = owner;
  op.chown.gid = group;

  link_debug(LINK_DEBUG_MESSAGE, "Write op");
  err = link_transport_masterwrite(driver, &op, sizeof(op));
  if (err < 0) {
    return err;
  }

  // Send the path on the bulk out endpoint
  link_debug(LINK_DEBUG_MESSAGE, "Write path");
  len = link_transport_masterwrite(driver, path, op.chown.path_size);
  if (len < 0) {
    return LINK_TRANSFER_ERR;
  }

  // read the reply to see if the file opened correctly
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to chown (%d)", link_errno);
  }

  return reply.err;
}

int link_chmod(link_transport_mdriver_t *driver, const char *path, int mode) {

  link_op_t op;
  link_reply_t reply;
  int len;
  int err;

  if (driver == NULL) {
    return -1;
  }

  link_debug(
    LINK_DEBUG_INFO, "call with (%s, %o) and handle %p", path, mode,
    driver->phy_driver.handle);

  if (strnlen(path, LINK_PATH_MAX_LARGE) > driver->path_max) {
    errno = ENAMETOOLONG;
    return -1;
  }

  op.chmod.cmd = LINK_CMD_CHMOD;
  op.chmod.path_size = strlen(path) + 1;
  op.chmod.mode = mode;

  link_debug(LINK_DEBUG_MESSAGE, "Write op");
  err = link_transport_masterwrite(driver, &op, sizeof(op));
  if (err < 0) {
    return err;
  }

  // Send the path on the bulk out endpoint
  link_debug(LINK_DEBUG_MESSAGE, "Write path");
  len = link_transport_masterwrite(driver, path, op.chown.path_size);
  if (len < 0) {
    return LINK_TRANSFER_ERR;
  }

  // read the reply to see if the file opened correctly
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to chmod (%d)", link_errno);
  }

  return reply.err;
}

/*! @} */
