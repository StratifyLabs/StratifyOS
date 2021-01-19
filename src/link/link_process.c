// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "link_local.h"

int link_exec(link_transport_mdriver_t *driver, const char *file) {
  link_op_t op;
  link_reply_t reply;
  int len;
  int err;

  link_debug(LINK_DEBUG_MESSAGE, "exec %s", file);

  op.exec.cmd = LINK_CMD_EXEC;
  op.exec.path_size = strnlen(file, LINK_ARG_MAX_LARGE);

  if (op.exec.path_size > driver->arg_max) {
    link_error(
      "Exec command size is too long %d > %d", op.exec.path_size, LINK_PATH_ARG_MAX);
    errno = ENAMETOOLONG;
    return -1;
  }

  link_debug(LINK_DEBUG_MESSAGE, "Write op (%p)", driver->phy_driver.handle);
  err = link_transport_masterwrite(driver, &op, sizeof(link_open_t));
  if (err < 0) {
    return err;
  }

  // Send the path on the bulk out endpoint
  link_debug(LINK_DEBUG_MESSAGE, "Write exec path (%d bytes)", op.exec.path_size);
  len = link_transport_masterwrite(driver, file, op.exec.path_size);
  if (len < 0) {
    link_error("Failed to write bulk output");
    return LINK_TRANSFER_ERR;
  }

  // read the reply to see if the file opened correctly
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_error("Failed to read the reply");
    return err;
  }

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to exec (%d)", link_errno);
  }

  return reply.err;
}

int link_kill_pid(link_transport_mdriver_t *driver, int pid, int signo) {
  int fd;
  int err_ioctl;
  int err;
  sys_killattr_t killattr;

  fd = link_open(driver, "/dev/sys", LINK_O_RDWR);
  if (fd < 0) {
    link_error("failed to open /dev/sys");
    return link_handle_err(driver, fd);
  }

  killattr.id = pid;
  killattr.si_signo = signo;
  killattr.si_sigcode = LINK_SI_USER;
  killattr.si_sigvalue = 0;

  err_ioctl = link_ioctl(driver, fd, I_SYS_KILL, &killattr);
  if (err_ioctl == LINK_PHY_ERROR) {
    link_error("failed to I_SYS_KILL");
    return err_ioctl;
  }

  if (err_ioctl == LINK_PROT_ERROR) {
    if (link_handle_err(driver, err_ioctl) == LINK_PHY_ERROR) {
      return LINK_PHY_ERROR;
    }
  }

  if ((err = link_close(driver, fd)) < 0) {
    link_error("failed to close fd");
    return err;
  }

  return err_ioctl;
}

/*! @} */
