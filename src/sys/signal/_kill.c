// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup signal
 * @{
 */

/*! \file */

#include <errno.h>
#include <signal.h>

#include "../scheduler/scheduler_local.h"
#include "sig_local.h"

/*! \details This function sends the signal \a signo to the process \a pid.
 *
 * \return Zero or -1 with errno (see \ref errno) set to:
 * - EINVAL: \a signo is not a valid signal number
 * - ESRCH: \a pid is not a valid process id
 *
 */
int kill(pid_t pid, int signo);

/*! \cond */
int _kill(pid_t pid, int signo) {
  int tid;

  for (tid = 1; tid < task_get_total(); tid++) {
    if ((pid == task_get_pid(tid)) && (task_thread_asserted(tid) == 0)) {
      break;
    }
  }

  if ((pid == task_get_pid(task_get_current())) && (signo == SIGABRT)) {
    sos_trace_event(LINK_POSIX_TRACE_FATAL, "sigabrt", sizeof("sigabrt"));
    sos_trace_stack((u32)-1);
  }

  //! \todo Add permission error checking
  return signal_send(tid, signo, SI_USER, 0);
}
/*! \endcond */

/*! @} */
