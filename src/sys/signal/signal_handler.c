// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup signal
 *
 * @{
 *
 */

/*! \file */

#include <signal.h>
#include <stdlib.h>

#include "cortexm/util.h"
#include "sig_local.h"
#include "sos/debug.h"
#include "trace.h"

#include "../scheduler/scheduler_root.h"
#include "../scheduler/scheduler_timing.h"


/*! \cond */
static void abort_action(int signo, int flags);
static void terminate_action(int signo, int flags);
static void stop_action(int signo, int flags);
static void continue_action(int signo, int flags);
static void ignore_action(int signo, int flags);

static void svcall_stoppid(void *args) MCU_ROOT_EXEC_CODE;
static void svcall_contpid(void *args) MCU_ROOT_EXEC_CODE;

static void (*const default_handlers[CONFIG_TASK_NUM_SIGNALS])(int, int) = {
  ignore_action,    // NULL SIGNAL
  abort_action,     // SIGABRT 1
  terminate_action, // SIGALRM 2
  abort_action,     // SIGBUS 3
  ignore_action,    // SIGCHLD 4
  continue_action,  // SIGCONT 5
  abort_action,     // SIGFPE 6
  terminate_action, // SIGHUP 7
  abort_action,     // SIGILL 8
  terminate_action, // SIGINT 9
  terminate_action, // SIGKILL 10
  terminate_action, // SIGPIPE 11
  abort_action,     // SIGQUIT 12
  abort_action,     // SIGSEGV 13
  stop_action,      // SIGSTOP 14
  terminate_action, // SIGTERM 15
  stop_action,      // SIGTSTP 16
  stop_action,      // SIGTTIN 17
  stop_action,      // SIGTTOU 18
  terminate_action, // SIGUSR1 19
  terminate_action, // SIGUSR2 20
  terminate_action, // SIGPOLL 21
  terminate_action, // SIGPROF 22
  abort_action,     // SIGSYS 23
  abort_action,     // SIGTRAP 24
  ignore_action,    // SIGURG 25
  terminate_action, // SIGVTALRM 26
  abort_action,     // SIGXCPU 27
  abort_action,     // SIGXFSZ 28
  ignore_action,    // SIGRTMIN 29
  ignore_action,    // SIGRT 30
  ignore_action     // SIGRTMAX 31
};

void signal_handler(int tid, int si_signo, int si_sigcode, union sigval sig_value) {
  siginfo_t siginfo;

  siginfo.si_signo = si_signo;
  siginfo.si_tid = tid;
  siginfo.si_code = si_sigcode;
  siginfo.si_value = sig_value;

  if (si_signo < CONFIG_TASK_NUM_SIGNALS) {
    struct sigaction *sa;
#if CONFIG_TASK_PROCESS_TIMER_COUNT > 0
    if (si_sigcode == SI_TIMER) {
      scheduler_timing_process_unqueue_timer(tid, si_signo, sig_value);
    }
#endif

    if (GLOBAL_SIGINFOS != NULL) {
      GLOBAL_SIGINFO(si_signo) = siginfo;
    }

    // This runs in user space to handle signals
    // Is signal being blocked? -- check the proc/thread signal mask to see if the signal
    // is blocked
    if ((si_signo != SIGSTOP) && (si_signo != SIGKILL)) {
      if (THREAD_SIGMASK & (1 << si_signo)) {
        // If the signal is blocked -- mark it as pending (unless it is ignored)
        if (GLOBAL_SIGACTIONS != NULL) {
          sa = GLOBAL_SIGACTION(si_signo);
          if (sa != NULL) {
            if (sa->sa_handler == SIG_IGN) { // Signal is ignored
              return;
            }
          } else {
            if (default_handlers[si_signo] == ignore_action) { // signal is ignored
              return;
            }
          }
        }

        THREAD_SIGPENDING |= (1 << si_signo);
        return;
      }
    }

    if (si_signo == SIGCHLD) {
      SIGCHLD_ASSERT();
    }

    // If the signal is not blocked call the handler
    if (GLOBAL_SIGACTIONS != NULL) {
      sa = GLOBAL_SIGACTION(si_signo);

      if (sa != NULL) {

        if (sa->sa_handler == SIG_DFL) {
          // call the default signal handler
          default_handlers[si_signo](si_signo, sa->sa_flags);
        } else if (sa->sa_handler == SIG_IGN) {
          ignore_action(si_signo, sa->sa_flags);
        } else if (sa->sa_flags & (1 << SA_SIGINFO)) {
          siginfo.si_signo = si_signo;
          sa->sa_sigaction(si_signo, &siginfo, NULL);
          SIGCAUGHT_ASSERT();
        } else {
          sa->sa_handler(si_signo);
          SIGCAUGHT_ASSERT();
        }

        return;
      }
    }

    // No custom handler is ready -- call the default handler
    default_handlers[si_signo](si_signo, 0);
  }
}

void abort_action(int signo, int flags) {
  MCU_UNUSED_ARGUMENT(flags);
  // Exit the program
  char hex_buffer[9];
  char str[32];
  strcpy(str, "ABORT:");
  htoa(hex_buffer, signo);
  strcat(str, hex_buffer);
  sos_trace_event(POSIX_TRACE_FATAL, str, strlen(str));
  _exit(signo << 8);
}

void terminate_action(int signo, int flags) {
  MCU_UNUSED_ARGUMENT(flags);
  // Exit the program
  char hex_buffer[9];
  char str[32];
  strcpy(str, "TERM:");
  htoa(hex_buffer, signo);
  strcat(str, hex_buffer);
  sos_trace_event(POSIX_TRACE_FATAL, str, strlen(str));
  _exit(signo << 8);
}

void svcall_stoppid(void *args) {
  CORTEXM_SVCALL_ENTER();
  MCU_UNUSED_ARGUMENT(args);
  int pid = task_get_pid(task_get_current());
  int i;

  i = task_get_current();
  if (task_thread_asserted(i)) {
    scheulder_root_assert_stopped(i);
  } else {
    for (i = 1; i < task_get_total(); i++) {
      if (task_get_pid(i) == pid) {
        scheulder_root_assert_stopped(i);
      }
    }
  }

  scheduler_root_update_on_stopped(); // causes the currently executing thread to sleep
}

void svcall_contpid(void *args) {
  CORTEXM_SVCALL_ENTER();
  MCU_UNUSED_ARGUMENT(args);
  int highest_prio = 0;
  int pid = task_get_pid(task_get_current());
  int i;

  i = task_get_current();

  if (task_thread_asserted(i)) {
    task_deassert_stopped(i);
    highest_prio = scheduler_priority(i);
  } else {
    for (i = 1; i < task_get_total(); i++) {
      if (task_get_pid(i) == pid) {
        scheduler_root_deassert_stopped(i);
        int prio = scheduler_priority(i);
        if (prio > highest_prio) {
          highest_prio = prio;
        }
      }
    }
  }

  scheduler_root_update_on_wake(-1, highest_prio);
}

void stop_action(int signo, int flags) {
  // stop all threads in the process
  if (task_get_current() == 0) { // can't stop the scheduler
    return;
  }

  if (signo == SIGCHLD) {
    if (!(flags & SA_NOCLDSTOP)) {
      // Send SIGCHLD to parent process (for wait and waitpid)

      // status for waitpid is 0x7F (see <sys/wait.h>)
    }
  }

  // Set the status of the process to WSTOPPED
  cortexm_svcall(svcall_stoppid, NULL);
}

void continue_action(int signo, int flags) {
  MCU_UNUSED_ARGUMENT(signo);
  MCU_UNUSED_ARGUMENT(flags);
  // continue this process
  if (task_get_pid(task_get_current()) == 0) {
    return;
  }

  // Set the status of the process to WCONTINUED
  cortexm_svcall(svcall_contpid, NULL);
}

void ignore_action(int signo, int flags) {
  MCU_UNUSED_ARGUMENT(signo);
  MCU_UNUSED_ARGUMENT(flags);

}
/*! \endcond */

/*! @} */
