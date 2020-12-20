// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file
 */

#include "../scheduler/scheduler_local.h"
#include "sos/debug.h"
#include "mcu/mcu.h"
#include "sos/fs/sysfs.h"
#include "sos/sos.h"
#include "unistd_fs.h"
#include "unistd_local.h"

/*! \details This function closes the file associated
 * with the specified descriptor.
 * \param fildes The File descriptor \a fildes.
 * \return Zero on success or -1 on error with errno (see \ref errno) set to:
 *  - EBADF:  Invalid file descriptor
 */
int close(int fildes);

/*! \cond */
int _close(int fildes) {
  scheduler_check_cancellation();
  // Close the file if it's open
  int ret;

  if (FILDES_IS_SOCKET(fildes)) {
    if (sos_config.socket_api != 0) {
      return sos_config.socket_api->close(fildes & ~FILDES_SOCKET_FLAG);
    }
    errno = EBADF;
    return -1;
  }

  fildes = u_fildes_is_bad(fildes);
  if (fildes < 0) {
    // check to see if fildes is a socket
    errno = EBADF;
    return -1;
  }

  ret = sysfs_file_close(get_open_file(fildes));
  u_reset_fildes(fildes);
  return ret;
}
/*! \endcond */

/*! @} */
