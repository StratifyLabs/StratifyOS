// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file */


#include <errno.h>
#include <unistd.h>

#include "../scheduler/scheduler_flags.h"
#include "cortexm/cortexm.h"
#include "cortexm/task.h"
#include "sos/sos.h"

static void svcall_update_task_root(void *args) {
  MCU_UNUSED_ARGUMENT(args);
  CORTEXM_SVCALL_ENTER();
  if (
    args != 0 &&
    // authenticated flag must be checked after CORTEXM_SVCALL_ENTER()
    scheduler_authenticated_asserted(task_get_current())) {
    task_assert_root(task_get_current());
  } else {
    task_deassert_root(task_get_current());
  }
}

/*! \details gets the effective user id.
 *
 *  The effectictive user id is not always the same as the user id.
 *  If the caller is authenticated, the user id will always be root and
 *  the effective user id can switch between root and user based on the usage
 *  `setuid()`.
 *
 *
 * \return 0 for root and 1 for user
 *
 */
uid_t geteuid() {
  return task_root_asserted(task_get_current()) ? SYSFS_ROOT : SYSFS_USER;
}

/*! \details gets the user id.
 *
 * If the process is authenticated, the user id is root (0) otherwise,
 * the user id is user (1).
 *
 * \return 0 for root and 1 for user
 *
 */
uid_t getuid() {
  return scheduler_authenticated_asserted(task_get_current()) ? SYSFS_ROOT : SYSFS_USER;
}

/*! \details sets the effective user id.
 *
 * \param uid The user id to set. Must be either SYSFS_USER or SYSFS_ROOT
 *
 * When the effective
 *
 * \return 0 on success or -1 with the errno set to:
 * - EPERM: process is not authenticated
 * - EINVAL: uid is not SYSFS_USER or SYSFS_ROOT
 *
 */
int seteuid(uid_t uid) {
  if (uid == SYSFS_USER) {
    // if root -- switch to user
    if (geteuid() == SYSFS_ROOT) {
      // deassert task root
      cortexm_svcall(svcall_update_task_root, NULL);
    }
    return 0;
  }

  if (uid == SYSFS_ROOT) {
    if (geteuid() == SYSFS_USER) {
      if (scheduler_authenticated_asserted(task_get_current())) {
        // assert task root
        cortexm_svcall(svcall_update_task_root, (void *)1);
        return 0;
      }
      errno = EPERM;
      return -1;
    }
    // already root
    return 0;
  }

  // only SOS_USER and SOS_USER_ROOT are valid user id values
  errno = EINVAL;
  return -1;
}

/*! \details functions exactly like `seteuid()`
 *
 */
int setuid(uid_t uid) { return seteuid(uid); }

// group is always 0
gid_t getgid() { return 0; }
gid_t getegid() { return 0; }

/*! @} */
