// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>
#include <unistd.h>

#include "../scheduler/scheduler_flags.h"
#include "cortexm/cortexm.h"
#include "cortexm/task.h"
#include "mcu/mcu.h"
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

uid_t geteuid() {
  return task_root_asserted(task_get_current()) ? SYSFS_ROOT : SYSFS_USER;
}

uid_t getuid() {
  return scheduler_authenticated_asserted(task_get_current()) ? SYSFS_ROOT : SYSFS_USER;
}

int seteuid(uid_t uid) {
  if (uid == SOS_USER) {
    // if root -- switch to user
    if (geteuid() == SOS_USER_ROOT) {
      // deassert task root
      cortexm_svcall(svcall_update_task_root, NULL);
    }
    return 0;
  }

  if (uid == SOS_USER_ROOT) {
    if (geteuid() == SOS_USER) {
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

int setuid(uid_t uid) { return seteuid(uid); }

// group is always 0
gid_t getgid() { return 0; }
gid_t getegid() { return 0; }
