// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup LINK
 * @{
 *
 */

//#include "config.h"

#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/fcntl.h> //Defines the flags

#include "cortexm/util.h"

#include "../process/process_start.h"
#include "../scheduler/scheduler_local.h"

#include "sos/debug.h"
#include "sos/fs.h"
#include "sos/sos.h"

#include "sos/link.h"
#include "trace.h"

#define SERIAL_NUM_WIDTH 3

/* The IMXRT USB driver is having problems
 * when writing two times in a row quickly.
 * This delay is inserted between consecutive writes.
 * It happens rarely so it has only
 * a negligible impact on performance for any platform.
 *
 */
#define BETWEEN_LINK_WRITE_DELAY() usleep(1000)

static int read_device(link_transport_driver_t *driver, int fildes, int size);
static int write_device(link_transport_driver_t *driver, int fildes, int size);
static int read_device_callback(void *context, void *buf, int nbyte);
static int write_device_callback(void *context, void *buf, int nbyte);
static void translate_link_stat(struct link_stat *dest, struct stat *src);

static int
read_path(link_transport_driver_t *driver, char *path, size_t size, size_t capacity);

typedef struct {
  link_op_t op;
  link_reply_t reply;
} link_data_t;

static void link_cmd_none(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_readserialno(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_ioctl(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_read(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_write(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_open(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_close(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_link(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_unlink(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_lseek(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_stat(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_fstat(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_mkdir(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_rmdir(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_opendir(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_readdir(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_closedir(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_rename(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_chown(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_chmod(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_exec(link_transport_driver_t *driver, link_data_t *args);
static void link_cmd_mkfs(link_transport_driver_t *driver, link_data_t *args);

void (*const link_cmd_func_table[LINK_CMD_TOTAL])(
  link_transport_driver_t *,
  link_data_t *) = {
  link_cmd_none,     link_cmd_readserialno, link_cmd_ioctl,   link_cmd_read,
  link_cmd_write,    link_cmd_open,         link_cmd_close,   link_cmd_link,
  link_cmd_unlink,   link_cmd_lseek,        link_cmd_stat,    link_cmd_fstat,
  link_cmd_mkdir,    link_cmd_rmdir,        link_cmd_opendir, link_cmd_readdir,
  link_cmd_closedir, link_cmd_rename,       link_cmd_chown,   link_cmd_chmod,
  link_cmd_exec,     link_cmd_mkfs};

void *link_update(void *arg) {
  int err;
  link_transport_driver_t *driver = arg;
  link_data_t data;
  data.op.cmd = 0;

  sos_debug_log_info(SOS_DEBUG_LINK, "Open link driver");
  if ((driver->handle = driver->open(NULL, 0)) == LINK_PHY_ERROR) {
    sos_debug_log_error(SOS_DEBUG_LINK, "failed to init phy");
    return 0;
  }

  sos_debug_log_directive(
    SOS_DEBUG_LINK,
    "sequenceDiagram:OS Link Messages:linkm:OS Device Host Message Sequences");
  sos_debug_log_datum(SOS_DEBUG_LINK, "linkm:participant H as host");
  sos_debug_log_datum(SOS_DEBUG_LINK, "linkm:participant D as device");

  sos_debug_log_info(SOS_DEBUG_LINK, "start link update");
  while (1) {

    // Wait for data to arrive on the link transport device
    while (1) {

      if (
        (err = link_transport_slaveread(driver, &data.op, sizeof(data.op), NULL, NULL))
        <= 0) {
        data.op = (link_op_t){};
        //very fast USB drivers (STM32H735) have trouble without a short delay here
        //need to give the host a little time to react
        usleep(100);
        sos_debug_log_warning(SOS_DEBUG_LINK, "slave read error %d", err);
        continue;
      }
      break;
    }
    if (data.op.cmd < LINK_CMD_TOTAL) {
      link_cmd_func_table[data.op.cmd](driver, &data);
    } else {
      data.reply.err = -1;
      data.reply.err_number = EINVAL;
    }

    // send the reply
    if (data.op.cmd != 0) {
      sos_debug_log_datum(
        SOS_DEBUG_LINK, "linkm:D->>H: Reply %d errno %d", data.reply.err,
        data.reply.err_number);
      link_transport_slavewrite(driver, &data.reply, sizeof(data.reply), NULL, NULL);
      data.op.cmd = 0;
    }
  }

  sos_debug_log_warning(SOS_DEBUG_LINK, "Link quit");
  return NULL;
}

void link_cmd_none(link_transport_driver_t *driver, link_data_t *args) {}

void link_cmd_isbootloader(link_transport_driver_t *driver, link_data_t *args) {
  args->reply.err = 0; // this is not the bootloader
}

static void svcall_get_serialno(void *dest) MCU_ROOT_EXEC_CODE;
void svcall_get_serialno(void *dest) {
  CORTEXM_SVCALL_ENTER();
  mcu_sn_t serial_number;
  int i, j;
  char *p = dest;
  sos_config.sys.get_serial_number(&serial_number);
  for (j = SERIAL_NUM_WIDTH; j >= 0; j--) {
    for (i = 0; i < 8; i++) {
      *p++ = htoc((serial_number.sn[j] >> 28) & 0x0F);
      serial_number.sn[j] <<= 4;
    }
  }
}

void link_cmd_readserialno(link_transport_driver_t *driver, link_data_t *args) {
  sos_debug_log_datum(SOS_DEBUG_LINK, "linkm:H->>D: read serial no");
  char serialno[LINK_PACKET_DATA_SIZE];
  memset(serialno, 0, LINK_PACKET_DATA_SIZE);
  cortexm_svcall(svcall_get_serialno, serialno);

  args->reply.err = strlen(serialno);
  args->reply.err_number = 0;

  if (
    link_transport_slavewrite(driver, &args->reply, sizeof(args->reply), NULL, NULL)
    < 0) {
    return;
  }

  BETWEEN_LINK_WRITE_DELAY();
  if (link_transport_slavewrite(driver, serialno, args->reply.err, NULL, NULL) < 0) {
    return;
  }

  args->op.cmd = 0; // reply was already sent
}

void link_cmd_open(link_transport_driver_t *driver, link_data_t *args) {
  errno = 0;
  char path[PATH_MAX + 1];
  if (read_path(driver, path, args->op.open.path_size, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }

  sos_debug_log_datum(SOS_DEBUG_LINK, "linkm:H->>D: open %s", path);
  args->reply.err = open(path, args->op.open.flags, args->op.open.mode);
  if (args->reply.err < 0) {
    sos_debug_log_error(SOS_DEBUG_LINK, "Failed to open %s (%d)", path, errno);
    args->reply.err_number = errno;
  }
}

void link_cmd_link(link_transport_driver_t *driver, link_data_t *args) {
  char old_path[PATH_MAX + 1];
  if (read_path(driver, old_path, args->op.symlink.path_size_old, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }

  char new_path[PATH_MAX + 1];
  if (read_path(driver, new_path, args->op.symlink.path_size_new, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }

  sos_debug_log_datum(SOS_DEBUG_LINK, "linkm:H->>D: link %s %s", old_path, new_path);
  args->reply.err = link(old_path, new_path);
  if (args->reply.err < 0) {
    sos_debug_log_error(SOS_DEBUG_LINK, "Failed to link %s (%d)", old_path, errno);
    args->reply.err_number = errno;
  }
}

void link_cmd_ioctl(link_transport_driver_t *driver, link_data_t *args) {
  errno = 0;
  const u16 size = _IOCTL_SIZE(args->op.ioctl.request);
  char io_buf[size];
  // this means data is being sent over to forward to ioctl
  if (_IOCTL_IOCTLW(args->op.ioctl.request) != 0) {
    if (link_transport_slaveread(driver, io_buf, size, NULL, NULL) < 0) {
      args->op.cmd = 0;
      args->reply.err = -1;
      return;
    }
  }

  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: ioctl %d 0x%08x", args->op.ioctl.fildes,
    args->op.ioctl.request);
  if (args->op.ioctl.fildes != driver->handle) {
    if (_IOCTL_IOCTLRW(args->op.ioctl.request) == 0) {
      // This means the third argument is just an integer
      args->reply.err =
        ioctl(args->op.ioctl.fildes, args->op.ioctl.request, args->op.ioctl.arg);
    } else {
      // This means a read or write is happening and the pointer should be passed
      args->reply.err = ioctl(args->op.ioctl.fildes, args->op.ioctl.request, io_buf);
    }
    args->reply.err_number = errno;
  } else {
    args->reply.err = -1;
    args->reply.err_number = EBADF;
  }

  // Check to see if this is a read operation and data must be sent back to the host
  if (_IOCTL_IOCTLR(args->op.ioctl.request) != 0) {
    // If the ioctl function reads data from the ctl argument, pass the data over the link
    if (link_transport_slavewrite(driver, io_buf, size, NULL, NULL) < 0) {
      sos_debug_log_error(SOS_DEBUG_LINK, "slave write failed");
      args->op.cmd = 0;
      args->reply.err = -1;
    }
    BETWEEN_LINK_WRITE_DELAY();
  }

  if (args->reply.err < 0) {
    if (args->op.ioctl.fildes != LINK_BOOTLOADER_FILDES) {
      sos_debug_log_error(SOS_DEBUG_LINK, "Failed to ioctl (%d)", errno);
    }
  }
}

void link_cmd_read(link_transport_driver_t *driver, link_data_t *args) {
  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: read fd=%d size=%d", args->op.read.fildes,
    args->op.read.nbyte);
  if (args->op.read.fildes != driver->handle) {
    errno = 0;
    args->reply.err = read_device(driver, args->op.read.fildes, args->op.read.nbyte);
  } else {
    args->reply.err = -1;
    args->reply.err_number = EBADF;
  }
  if (args->reply.err < 0) {
    sos_debug_log_error(SOS_DEBUG_LINK, "Failed to read (%d)", errno);
    args->reply.err_number = errno;
  }
}

void link_cmd_write(link_transport_driver_t *driver, link_data_t *args) {
  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: write fd=%d size=%d", args->op.write.fildes,
    args->op.write.nbyte);
  if (args->op.write.fildes != driver->handle) {
    errno = 0;
    args->reply.err = write_device(driver, args->op.write.fildes, args->op.write.nbyte);
  } else {
    args->reply.err = -1;
    args->reply.err_number = EBADF;
  }
  if (args->reply.err < 0) {
    args->reply.err_number = errno;
    sos_debug_log_error(
      SOS_DEBUG_LINK, "Failed to write (%d, %d)", args->reply.err,
      args->reply.err_number);
  }
}

void link_cmd_close(link_transport_driver_t *driver, link_data_t *args) {
  sos_debug_log_datum(SOS_DEBUG_LINK, "linkm:H->>D: close fd=%d", args->op.write.fildes);
  if (args->op.ioctl.fildes != driver->handle) {
    args->reply.err = close(args->op.close.fildes);
  } else {
    args->reply.err = -1;
    args->reply.err_number = EBADF;
  }
  if (args->reply.err < 0) {
    sos_debug_log_error(SOS_DEBUG_LINK, "Failed to close (%d)", errno);
    args->reply.err_number = errno;
  }
}

void link_cmd_unlink(link_transport_driver_t *driver, link_data_t *args) {
  char path[PATH_MAX + 1];
  if (read_path(driver, path, args->op.unlink.path_size, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }
  sos_debug_log_datum(SOS_DEBUG_LINK, "linkm:H->>D: unlink %s", path);
  args->reply.err = unlink(path);
  if (args->reply.err < 0) {
    sos_debug_log_error(SOS_DEBUG_LINK, "Failed to unlink (%d)", errno);
    args->reply.err_number = errno;
  }
}

void link_cmd_lseek(link_transport_driver_t *driver, link_data_t *args) {
  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: lseek fd=%d whence=%d offset=%d", args->op.lseek.fildes,
    args->op.lseek.whence, args->op.lseek.offset);
  args->reply.err =
    lseek(args->op.lseek.fildes, args->op.lseek.offset, args->op.lseek.whence);
  if (args->reply.err < 0) {
    args->reply.err_number = errno;
  }
}

void link_cmd_stat(link_transport_driver_t *driver, link_data_t *args) {
  int err;
  struct stat st;
  struct link_stat lst;

  char path[PATH_MAX + 1];
  if (read_path(driver, path, args->op.stat.path_size, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }
  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: stat %s", path);
  args->reply.err = stat(path, &st);
  if (args->reply.err < 0) {
    sos_debug_log_datum(
      SOS_DEBUG_LINK, "linkm:D->>H: failed");
    sos_debug_log_error(SOS_DEBUG_LINK, "Failed to stat (%d)", errno);
    args->reply.err_number = errno;
  } else {
    sos_debug_log_datum(
      SOS_DEBUG_LINK, "linkm:D->>H: size=%d mode=0%o", st.st_size, st.st_mode);
  }

  translate_link_stat(&lst, &st);
  args->op.cmd = 0;

  // Send the reply
  link_transport_slavewrite(driver, &args->reply, sizeof(link_reply_t), NULL, NULL);
  if (args->reply.err < 0) {
    return;
  }

  // now send the data
  BETWEEN_LINK_WRITE_DELAY();
  err = link_transport_slavewrite(driver, &lst, sizeof(struct link_stat), NULL, NULL);
  if (err < -1) {
    return;
  }
}

void link_cmd_fstat(link_transport_driver_t *driver, link_data_t *args) {
  int err;
  struct stat st;
  struct link_stat lst;

  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: fstat fd=%d", args->op.fstat.fildes);

  args->reply.err = fstat(args->op.fstat.fildes, (struct stat *)&st);
  if (args->reply.err < 0) {
    args->reply.err_number = errno;
  }

  translate_link_stat(&lst, &st);
  args->op.cmd = 0;

  // Send the reply
  link_transport_slavewrite(driver, &args->reply, sizeof(link_reply_t), NULL, NULL);
  if (args->reply.err < -1) {
    return;
  }

  // now send the data
  BETWEEN_LINK_WRITE_DELAY();
  err = link_transport_slavewrite(driver, &lst, sizeof(struct link_stat), NULL, NULL);
  if (err < -1) {
    return;
  }
}

void link_cmd_mkdir(link_transport_driver_t *driver, link_data_t *args) {
  char path[PATH_MAX + 1];
  if (read_path(driver, path, args->op.mkdir.path_size, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }

  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: mkdir %s 0%o", path, args->op.mkdir.mode);
  args->reply.err = mkdir(path, args->op.mkdir.mode);
  if (args->reply.err < 0) {
    args->reply.err_number = errno;
  }
}

void link_cmd_rmdir(link_transport_driver_t *driver, link_data_t *args) {
  char path[PATH_MAX + 1];
  if (read_path(driver, path, args->op.rmdir.path_size, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }
  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: rmdir %s", path);
  args->reply.err = rmdir(path);
  if (args->reply.err < 0) {
    args->reply.err_number = errno;
  }
}

void link_cmd_opendir(link_transport_driver_t *driver, link_data_t *args) {
  char path[PATH_MAX + 1];
  if (read_path(driver, path, args->op.opendir.path_size, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }

  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: opendir %s", path);
  args->reply.err = (int)opendir(path);
  if (args->reply.err == 0) {
    sos_debug_log_error(SOS_DEBUG_LINK, "Failed to open dir %s (%d)", path, errno);
    args->reply.err_number = errno;
  }
}

void link_cmd_readdir(link_transport_driver_t *driver, link_data_t *args) {
  struct dirent de;
  struct link_dirent lde;

  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: readdir dirp=%p", args->op.readdir.dirp);

  args->reply.err = readdir_r((DIR *)args->op.readdir.dirp, &de, NULL);

  if (args->reply.err < 0) {
    args->reply.err = -1;
    args->reply.err_number = errno;
    if (errno != 2) {
      sos_debug_log_error(SOS_DEBUG_LINK, "Failed to read dir (%d)", errno);
    }
    sos_debug_log_datum(
      SOS_DEBUG_LINK, "linkm:D->>H: end of directory");
  } else {
    sos_debug_log_datum(
      SOS_DEBUG_LINK, "linkm:D->>H: %s", de.d_name);
  }

  args->op.cmd = 0;

  lde.d_ino = de.d_ino;
  strcpy(lde.d_name, de.d_name);


  if (
    link_transport_slavewrite(driver, &args->reply, sizeof(link_reply_t), NULL, NULL)
    < 0) {
    return;
  }

  if (args->reply.err < 0) {
    return;
  }

  BETWEEN_LINK_WRITE_DELAY();
  link_transport_slavewrite(driver, &lde, sizeof(struct link_dirent), NULL, NULL);
}

void link_cmd_closedir(link_transport_driver_t *driver, link_data_t *args) {
  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: closedir dirp=%p", args->op.closedir.dirp);
  args->reply.err = closedir((DIR *)args->op.closedir.dirp);
  if (args->reply.err < 0) {
    args->reply.err_number = errno;
  }
}

void link_cmd_rename(link_transport_driver_t *driver, link_data_t *args) {

  char old_path[PATH_MAX + 1];
  if (read_path(driver, old_path, args->op.rename.old_size, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }

  char new_path[PATH_MAX + 1];
  if (read_path(driver, new_path, args->op.rename.new_size, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }

  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: rename %s to %s", old_path, new_path);
  args->reply.err = rename(old_path, new_path);
  if (args->reply.err < 0) {
    args->reply.err_number = errno;
  }
}

void link_cmd_chown(link_transport_driver_t *driver, link_data_t *args) {
  char path[PATH_MAX + 1];
  if (read_path(driver, path, args->op.chown.path_size, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }

  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: chown %s uid=%d gid=%d", path, args->op.chown.uid, args->op.chown.gid);

  args->reply.err = chown(path, args->op.chown.uid, args->op.chown.gid);
  if (args->reply.err < 0) {
    args->reply.err_number = errno;
  }
}

void link_cmd_chmod(link_transport_driver_t *driver, link_data_t *args) {
  char path[PATH_MAX + 1];
  if (read_path(driver, path, args->op.chmod.path_size, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }

  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: chmod %s mode=0%o gid=%d", path, args->op.chmod.mode);

  args->reply.err = chmod(path, args->op.chmod.mode);
  if (args->reply.err < 0) {
    args->reply.err_number = errno;
  }
}

void link_cmd_exec(link_transport_driver_t *driver, link_data_t *args) {
  // was LINK_PATH_ARG_MAX
  char path_arg[ARG_MAX + 1];
  if (read_path(driver, path_arg, args->op.exec.path_size, ARG_MAX) < 0) {
    sos_debug_log_error(SOS_DEBUG_LINK, "Failed to read path", path_arg, errno);
    driver->flush(driver->handle);
    return;
  }


  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: exec %s", path_arg);

  args->reply.err = process_start(path_arg, NULL);
  if (args->reply.err < 0) {
    sos_debug_log_error(SOS_DEBUG_LINK, "Failed to exec %s (%d)", path_arg, errno);
    args->reply.err_number = errno;
  }
}

void link_cmd_mkfs(link_transport_driver_t *driver, link_data_t *args) {
  char path[PATH_MAX + 1];
  if (read_path(driver, path, args->op.mkfs.path_size, PATH_MAX) < 0) {
    driver->flush(driver->handle);
    return;
  }

  // send the reply immediately so that the protocol isn't left waiting for a lengthy
  // format
  args->reply.err = 0;
  errno = EBUSY;
  args->reply.err_number = errno;

  link_transport_slavewrite(driver, &args->reply, sizeof(args->reply), NULL, NULL);

  // tells the caller not to send the reply later
  args->op.cmd = 0;

  sos_debug_log_datum(
    SOS_DEBUG_LINK, "linkm:H->>D: mkfs %s", path);

  args->reply.err = mkfs(path);
  if (args->reply.err < 0) {
    args->reply.err_number = errno;
  } else {
    errno = 0;
  }
}

int read_device_callback(void *context, void *buf, int nbyte) {
  int *fildes;
  int ret;
  fildes = context;
  ret = read(*fildes, buf, nbyte);
  return ret;
}

int write_device_callback(void *context, void *buf, int nbyte) {
  int *fildes;
  int ret;
  fildes = context;
  ret = write(*fildes, buf, nbyte);
  return ret;
}

int read_device(link_transport_driver_t *driver, int fildes, int nbyte) {
  return link_transport_slavewrite(driver, NULL, nbyte, read_device_callback, &fildes);
}

int write_device(link_transport_driver_t *driver, int fildes, int nbyte) {
  return link_transport_slaveread(driver, NULL, nbyte, write_device_callback, &fildes);
}

int read_path(link_transport_driver_t *driver, char *path, size_t size, size_t capacity) {
  int result;
  if (size > capacity) {
    // path should never be larger than PATH_MAX
    return -1;
  } else {
    // just read into the buffer
    result = link_transport_slaveread(driver, path, size, NULL, NULL);
  }
  // ensure null termination
  path[size] = 0;
  return result;
}

void translate_link_stat(struct link_stat *dest, struct stat *src) {
  dest->st_dev = src->st_dev;
  dest->st_ino = src->st_ino;
  dest->st_mode = src->st_mode;
  dest->st_uid = src->st_uid;
  dest->st_gid = src->st_gid;
  dest->st_rdev = src->st_rdev;
  dest->st_size = src->st_size;
  dest->st_mtime_ = src->st_mtime;
  dest->st_ctime_ = src->st_ctime;
  dest->st_blksize = src->st_blksize;
  dest->st_blocks = src->st_blocks;
}

/*! @} */
