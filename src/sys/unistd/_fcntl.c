// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include <fcntl.h>
#include <stdarg.h>

#include "../scheduler/scheduler_local.h"
#include "sos/sos.h"
#include "unistd_fs.h"
#include "unistd_local.h"

#include "sos/debug.h"

/*! \details This function performs various operations on open files such as:
 * - F_DUPFD: duplicate a file descriptor
 * - F_GETFD:  get the file descriptor flags
 * - F_SETFD:  set the file descriptor flags
 * - F_GETOWN: get the file descriptor owner process ID.
 *
 * \param fildes The file descriptor
 * \param cmd The operation to perform
 *
 * \return Zero on success or -1 on error with errno (see \ref errno) set to:
 *  - EBADF:  invalid file descriptor
 *  - ENOTSUP:  \a cmd is not supported for the file descriptor
 *
 */
int fcntl(int fildes, int cmd, ...);

/*! \cond */
int _fcntl(int fildes, int cmd, ...) {
  int tmp;
  int flags;
  va_list ap;

  va_start(ap, cmd);
  tmp = va_arg(ap, int);
  va_end(ap);

  scheduler_check_cancellation();

  if (FILDES_IS_SOCKET(fildes)) {
    if (sos_config.socket_api != 0) {
      return sos_config.socket_api->fcntl(fildes & ~FILDES_SOCKET_FLAG, cmd, tmp);
    }
    errno = EBADF;
    return SYSFS_RETURN_LINE();
  }

  fildes = u_fildes_is_bad(fildes);
  if (fildes < 0) {
    errno = EBADF;
    return SYSFS_RETURN_LINE();
  }

  flags = get_flags(fildes);

  switch (cmd) {
    /*
    case F_DUPFD:

            new_fildes = u_new_open_file(tmp);
            if ( new_fildes == -1 ){
                    return -1;
            }

            dup_open_file(new_fildes, fildes);
            return new_fildes;
            */

  case F_GETFL:
    return flags;

  case F_SETFL:
    set_flags(fildes, tmp);
    break;

  case F_GETFD:
  case F_SETFD:
  case F_DUPFD:
  case F_GETOWN:
  case F_SETOWN:
  case F_GETLK:
  case F_SETLK:
  case F_SETLKW:
    errno = ENOTSUP;
    break;
  default:
    errno = EINVAL;
    break;
  }

  return -1;
}
/*! \endcond */

/*! @} */
