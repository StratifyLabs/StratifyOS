/* Copyright 2011-2016 Tyler Gilbert;
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "link_local.h"

// Access to directories
int link_mkdir(link_transport_mdriver_t *driver, const char *path, int mode) {
  link_op_t op;
  link_reply_t reply;
  int len;
  int err;

  if (driver == NULL) {
    return mkdir(
      path
#if !defined __win32
      ,
      mode
#endif
    );
  }

  link_debug(
    LINK_DEBUG_INFO, "call with (%s, 0%o) and handle %p", path, mode,
    driver->phy_driver.handle);

  op.mkdir.cmd = LINK_CMD_MKDIR;
  op.mkdir.mode = mode;
  op.mkdir.path_size = strlen(path) + 1;

  link_debug(LINK_DEBUG_MESSAGE, "Write op");
  err = link_transport_masterwrite(driver, &op, sizeof(op));
  if (err < 0) {
    return err;
  }

  // Send the path on the bulk out endpoint
  link_debug(LINK_DEBUG_MESSAGE, "Write path");
  len = link_transport_masterwrite(driver, path, op.mkdir.path_size);
  if (len < 0) {
    return LINK_TRANSFER_ERR;
  }

  // read the reply to see if the file opened correctly
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_error("Failed to get reply");
    return err;
  }

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to mkdir (%d)", link_errno);
  }

  return reply.err;
}

int link_rmdir(link_transport_mdriver_t *driver, const char *path) {
  link_op_t op;
  link_reply_t reply;
  int len;
  int err;

  if (driver == NULL) {
    return rmdir(path);
  }

  link_debug(
    LINK_DEBUG_INFO, "call with (%s) and handle %p", path, driver->phy_driver.handle);

  op.rmdir.cmd = LINK_CMD_RMDIR;
  op.rmdir.path_size = strlen(path) + 1;

  link_debug(LINK_DEBUG_MESSAGE, "Write op");
  err = link_transport_masterwrite(driver, &op, sizeof(op));
  if (err < 0) {
    link_error("Failed to write op");
    return err;
  }

  // Send the path on the bulk out endpoint
  link_debug(LINK_DEBUG_MESSAGE, "Write path");
  len = link_transport_masterwrite(driver, path, op.rmdir.path_size);
  if (len < 0) {
    link_error("Failed to write path");
    return LINK_TRANSFER_ERR;
  }

  // read the reply to see if the file opened correctly
  link_debug(LINK_DEBUG_MESSAGE, "read reply");
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_error("Failed to read reply");
    return err;
  }

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to rmdir (%d)", link_errno);
  }

  return reply.err;
}

DIR *link_opendir(link_transport_mdriver_t *driver, const char *dirname) {
  link_op_t op;
  link_reply_t reply;
  int len;
  int err;

  if (driver == NULL) {
    return opendir(dirname);
  }

  link_debug(
    LINK_DEBUG_INFO, "call with (%s) and handle %p", dirname, driver->phy_driver.handle);

  op.opendir.cmd = LINK_CMD_OPENDIR;
  op.opendir.path_size = strlen(dirname) + 1;

  if (dirname == NULL) {
    link_error("Directory name is NULL");
    return 0;
  }

  link_debug(LINK_DEBUG_MESSAGE, "Write op");
  err = link_transport_masterwrite(driver, &op, sizeof(link_opendir_t));
  if (err < 0) {
    link_error("Failed to transfer command");
    return 0;
  }

  // Send the path on the bulk out endpoint
  link_debug(LINK_DEBUG_MESSAGE, "Write path %s", dirname);
  len = link_transport_masterwrite(driver, dirname, op.opendir.path_size);
  if (len < 0) {
    link_error("Failed to write bulk out");
    return 0;
  }

  link_debug(LINK_DEBUG_MESSAGE, "Write path len is %d 0x%X", len, reply.err);
  // read the reply to see if the file opened correctly
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_error("Failed to read bulk in");
    return 0;
  }

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to opendir (%d)", link_errno);
  } else {
    link_debug(LINK_DEBUG_INFO, "new dirp is 0x%X", reply.err);
  }
  return (DIR *)((size_t)reply.err);
}

int link_readdir_r(
  link_transport_mdriver_t *driver,
  DIR *dirp,
  struct dirent *entry,
  struct dirent **result) {
  link_op_t op;
  link_reply_t reply;
  int len;

  if (driver == NULL) {
    struct dirent *result_dirent = readdir(dirp);
    if (result_dirent) {
      memcpy(entry, result_dirent, sizeof(struct dirent));
      if (result != NULL) {
        *result = entry;
        return 0;
      }
    }
    return -1;
  }

  if (result != NULL) {
    *result = NULL;
  }

  link_debug(
    LINK_DEBUG_INFO, "call with (0x%X, %p) and handle %p", dirp, entry,
    driver->phy_driver.handle);

  op.readdir.cmd = LINK_CMD_READDIR;
  op.readdir.dirp = (u32)(u64)dirp;

  link_debug(LINK_DEBUG_MESSAGE, "Write op");
  if (link_transport_masterwrite(driver, &op, sizeof(link_readdir_t)) < 0) {
    return -1;
  }

  reply.err = 0;
  reply.err_number = 0;
  link_debug(LINK_DEBUG_MESSAGE, "Read reply size=%ld", sizeof(reply));
  if (link_transport_masterread(driver, &reply, sizeof(reply)) < 0) {
    return -1;
  }

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to readdir (%d)", link_errno);
    return reply.err;
  }

  struct link_dirent link_entry;
  // Read the bulk in buffer for the result of the read
  link_debug(LINK_DEBUG_MESSAGE, "Read link dirent");
  len = link_transport_masterread(driver, &link_entry, sizeof(struct link_dirent));
  if (len < 0) {
    link_error("Failed to read dirent");
    return -1;
  }

  if (result != NULL) {
    *result = entry;
  }

  memset(entry, 0, sizeof(struct dirent));
  strncpy(entry->d_name, link_entry.d_name, LINK_NAME_MAX);
  entry->d_ino = link_entry.d_ino;

  return 0;
}

int link_closedir(link_transport_mdriver_t *driver, DIR *dirp) {
  link_op_t op;
  link_reply_t reply;
  int err;

  if (driver == NULL) {
    return closedir(dirp);
  }

  link_debug(
    LINK_DEBUG_INFO, "call with (0x%X) and handle %p", dirp, driver->phy_driver.handle);

  op.closedir.cmd = LINK_CMD_CLOSEDIR;
  op.closedir.dirp = (u32)(u64)dirp;

  link_debug(LINK_DEBUG_MESSAGE, "Write op");
  err = link_transport_masterwrite(driver, &op, sizeof(link_closedir_t));
  if (err < 0) {
    link_error("Failed to write op");
    return err;
  }

  link_debug(LINK_DEBUG_MESSAGE, "Read Reply");
  err = link_transport_masterread(driver, &reply, sizeof(reply));
  if (err < 0) {
    link_error("Failed to read reply");
    return err;
  }

  if (reply.err < 0) {
    link_errno = reply.err_number;
    link_debug(LINK_DEBUG_WARNING, "Failed to closedir (%d)", link_errno);
  }
  return reply.err;
}

int link_seekdir(link_transport_mdriver_t *driver, DIR *dirp, int location) {
  if (driver == NULL) {
    seekdir(dirp, location);
    return 0;
  }
  return -1;
}

int link_telldir(link_transport_mdriver_t *driver, DIR *dirp) {
  if (driver == NULL) {
    return telldir(dirp);
  }
  return -1;
}

int link_rewinddir(link_transport_mdriver_t *driver, DIR *dirp) {
  if (driver == NULL) {
    rewinddir(dirp);
    return 0;
  }
  return -1;
}
