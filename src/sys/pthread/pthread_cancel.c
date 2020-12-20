// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup pthread
 * @{
 *
 */

/*! \file */

#include "config.h"

#include <errno.h>
#include <pthread.h>

#include "../scheduler/scheduler_flags.h"
#include "../scheduler/scheduler_local.h"
#include "cortexm/task.h"

typedef struct {
  int tid;
  int cancel;
  int enable;
  int asynchronous;
  int old_state;
  int old_type;
} svcall_cancel_update_t;

static void svcall_cancel_update(void *args) MCU_ROOT_EXEC_CODE;

void svcall_cancel_update(void *args) {
  CORTEXM_SVCALL_ENTER();
  svcall_cancel_update_t *p = args;
  int tid = p->tid;

  if (scheduler_cancel_asynchronous_asserted(tid)) {
    p->old_type = PTHREAD_CANCEL_ASYNCHRONOUS;
  } else {
    p->old_type = PTHREAD_CANCEL_DEFERRED;
  }

  if (scheduler_cancel_enable_asserted(tid)) {
    p->old_state = PTHREAD_CANCEL_ENABLE;
  } else {
    p->old_state = PTHREAD_CANCEL_DISABLE;
  }

  if ((p->cancel > 0) && scheduler_cancel_enable_asserted(tid)) {
    scheduler_root_assert_cancel(tid);
  } else if (p->cancel < 0) {
    scheduler_root_deassert_cancel(tid);
  }

  if (p->enable > 0) {
    scheduler_root_assert_cancel_enable(tid);
  } else if (p->enable < 0) {
    scheduler_root_deassert_cancel_enable(tid);
  }

  if (p->asynchronous > 0) {
    scheduler_root_assert_cancel_asynchronous(tid);
  } else if (p->asynchronous < 0) {
    scheduler_root_deassert_cancel_asynchronous(tid);
  }
}

/*! \details This function cancels the specified thread.
 * \return Zero on success or -1 with errno set.
 *
 *
 */
int pthread_cancel(pthread_t thread) {
  int pid = task_get_pid(task_get_current());
  int thread_pid = task_get_pid(thread);

  if (
    ((u8)thread >= task_get_total()) || (pid != thread_pid)
    || (task_thread_asserted(thread) == 0) || (task_enabled(thread) == 0)) {
    errno = ESRCH;
    return -1;
  }

  svcall_cancel_update_t update = {0};
  update.tid = thread;
  update.cancel = 1;
  cortexm_svcall(svcall_cancel_update, &update);
  return 0;
}

/*! \details This function is not supported.
 * \return -1 with errno equal to ENOTSUP
 */
int pthread_setcancelstate(int state, int *oldstate) {
  // PTHREAD_CANCEL_ENABLE or PTHREAD_CANCEL_DISABLE
  svcall_cancel_update_t update = {0};
  update.tid = task_get_current();
  if (state == PTHREAD_CANCEL_ENABLE) {
    update.enable = 1;
  } else if (state == PTHREAD_CANCEL_DISABLE) {
    update.enable = -1;
  } else {
    errno = EINVAL;
    return -1;
  }
  cortexm_svcall(svcall_cancel_update, &update);

  if (oldstate != 0) {
    *oldstate = update.old_state;
  }

  if (state == PTHREAD_CANCEL_ENABLE) {
    scheduler_check_cancellation();
  }

  return 0;
}

/*! \details This function is not supported.
 * \return -1 with errno equal to ENOTSUP
 */
int pthread_setcanceltype(int type, int *oldtype) {
  // PTHREAD_CANCEL_DEFERRED or PTHREAD_CANCEL_ASYNCHRONOUS

  svcall_cancel_update_t update = {0};
  update.tid = task_get_current();
  if (type == PTHREAD_CANCEL_ASYNCHRONOUS) {
    // update.asynchronous = 1;
    // for now don't support this -- too many problems
    errno = EINVAL;
    return -1;
  } else if (type == PTHREAD_CANCEL_DEFERRED) {
    update.asynchronous = -1;
  } else {
    errno = EINVAL;
    return -1;
  }

  cortexm_svcall(svcall_cancel_update, &update);
  if (oldtype != NULL) {
    *oldtype = update.old_type;
  }

  return 0;
}

void pthread_testcancel() {
  // TODO check if thread is asynchronous cancellable
  scheduler_check_cancellation();
}

/*! @} */
