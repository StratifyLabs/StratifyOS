// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include "config.h"
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/reent.h>
#include <unistd.h>

#include "../scheduler/scheduler_root.h"
#include "../signal/sig_local.h"
#include "cortexm/mpu.h"
#include "sos/dev/appfs.h"
#include "sos/fs/sysfs.h"

/*! \cond */
static void svcall_stop_threads(int *send_signal) MCU_ROOT_EXEC_CODE;
static void svcall_zombie_process(int *signal_sent) MCU_ROOT_EXEC_CODE;
/*! \endcond */

/*! \details This function causes the calling process
 * to exit with the specified exit code.
 *
 * \note In this version, named semaphores are not closed in this function.
 *
 * \return This function never returns
 */
void _exit(int __status) {
  int send_signal;
  int tmp;
  int i;

  // todo close named semaphores

  // tell free to skip actually freeing memory because the app is shutting down
  // anyway
  _GLOBAL_REENT->procmem_base->size = 0;

  // Make sure all open file descriptors are closed
  for (i = 0; i < OPEN_MAX; i++) {
    close(i); // make sure all file descriptors are closed
  }

  // unlock any locks on the filesystems
  sysfs_unlock();

  tmp = (__status >> 8) & 0xff;  // the signal number if terminated by a signal
  send_signal = __status & 0xff; // this is the 8-bit exit code
  __status = (send_signal << 8) | tmp;
  send_signal = __status;

  cortexm_svcall((cortexm_svcall_t)svcall_stop_threads, &send_signal);

  while (1) { // if the process turns in to a zombie it should resume zombie mode if it
              // receives a signal
    // This process will be a zombie process until it is disabled by the parent
    cortexm_svcall((cortexm_svcall_t)svcall_zombie_process, &send_signal);
  }
}

/*! \cond */
void svcall_stop_threads(int *send_signal) {
  CORTEXM_SVCALL_ENTER();
  int i;
  int tmp;
  struct _reent *parent_reent;

  // set the exit status
  sos_sched_table[task_get_current()].exit_status = *send_signal;

  // Kill all other threads in process
  tmp = task_get_pid(task_get_current());
  // Terminate the threads in this process
  for (i = 1; i < task_get_total(); i++) {
    if (task_get_pid(i) == tmp) {
      if (i != task_get_current()) {
        sos_sched_table[i].flags = 0;
        task_root_delete(i);
      }
    }
  }

  // now check for SA_NOCLDWAIT in parent process
  tmp = task_get_parent(task_get_current());
  parent_reent = (struct _reent *)sos_task_table[tmp].global_reent;

  if (task_get_pid(tmp) == 0) {
    // process 0 never waits - so don't send it a signal
    *send_signal = false;
  } else {
    *send_signal = true;
    if (parent_reent->procmem_base->sigactions != NULL) {
      if (parent_reent->procmem_base->sigactions[SIGCHLD] != NULL) {
        if (
          (parent_reent->procmem_base->sigactions[SIGCHLD]->sa_flags
           & (1 << SA_NOCLDWAIT))
          || parent_reent->procmem_base->sigactions[SIGCHLD]->sa_handler == SIG_IGN) {
          // do not send SIGCHLD -- do not be a zombie process
          *send_signal = false;
        }
      }
    }
  }

  if (scheduler_zombie_asserted(task_get_current())) {
    // This will be executed if a SIGTERM is sent to a ZOMBIE process
    *send_signal = false;
  } else if (*send_signal == true) {
    // assert the zombie flags

    // send a signal to the parent process
    tmp = task_get_pid(task_get_parent(task_get_current()));
    int options = 0;
    for (i = 0; i < task_get_total(); i++) {
      // send SIGCHLD to each thread of parent
      if ((tmp == task_get_pid(i)) && (task_enabled(i))) {
        options = 1;
        errno = 0;
        if (
          signal_root_send(
            task_get_current(), i, SIGCHLD, SI_USER,
            sos_sched_table[task_get_current()].exit_status, 0)
          < 0) {
          *send_signal = false;
          return;
        }
      }
    }

    if (options == 0) {
      // signal was never sent -- no matching parent
      *send_signal = false;
    }

    sos_sched_table[task_get_current()].flags |= (1 << SCHEDULER_TASK_FLAG_ZOMBIE);

  }
}

void svcall_zombie_process(int *signal_sent) {
  CORTEXM_SVCALL_ENTER();
  if (*signal_sent == false) {
    // discard this thread immediately
    sos_sched_table[task_get_current()].flags = 0;
    task_root_delete(task_get_current());
  } else {
    // the parent is waiting -- set this thread to a zombie thread
    scheduler_root_deassert_inuse(task_get_current());
  }

  scheduler_root_update_on_sleep();
}
/*! \endcond */

/*! @} */
