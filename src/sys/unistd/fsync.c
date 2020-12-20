// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include <errno.h>
#include <stdarg.h>

#include "config.h"

#include "../scheduler/scheduler_local.h"
#include "device/sys.h"
#include "mcu/core.h"
#include "sos/sos.h"
#include "unistd_local.h"

/*! \details This function performs a control request on the device
 * associated with \a fildes. \a request is specific to the device.
 * The value of \a request determines what value should be passed
 * as the \a ctl argument.
 *
 * \param fildes The file descriptor returned by \ref open()
 *
 * \return The number of bytes actually read of -1 with errno (see \ref errno) set to:
 * - EBADF:  \a fildes is bad
 * - EIO:  IO error
 * - EAGAIN:  O_NONBLOCK is set for \a fildes and the device is busy
 *
 */
int fsync(int fildes) {
  scheduler_check_cancellation();

  fildes = u_fildes_is_bad(fildes);
  if (fildes < 0) {
    // check to see if fildes is a socket
    errno = EBADF;
    return -1;
  }

  if (FILDES_IS_SOCKET(fildes)) {
    if (sos_config.socket_api != 0) {
      return sos_config.socket_api->fsync(fildes);
    }
    errno = EBADF;
    return -1;
  }

  return sysfs_file_fsync(get_open_file(fildes));
}

/*! @} */
