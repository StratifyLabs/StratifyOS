// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup time
 * @{
 */

/*! \file */

#include "config.h"
#include <errno.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

#include "../scheduler/scheduler_timing.h"

/*! \cond */
#define CLOCK_PROCESS_FLAG (1 << 31)
static s32 convert_clocks_to_nanoseconds(s32 clocks);
static void task_timer_to_timespec(struct timespec *tp, u64 task_timer);

#define SCHEDULER_CLOCK_NSEC_DIV                                                         \
  ((u32)((u64)1024 * 1000000000 / sos_config.sys.core_clock_frequency))

/*! \endcond */

/*! \details This function is not supported.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - ENOTSUP:  not supported
 */
int clock_getcpuclockid(pid_t pid, clockid_t *clock_id) {
  MCU_UNUSED_ARGUMENT(pid);
  MCU_UNUSED_ARGUMENT(clock_id);
  errno = ENOTSUP;
  return -1;
}

/*! \details This function gets the time of the \a id clock where \a id is one of:
 * - CLOCK_MONOTONIC
 * - CLOCK_REALTIME
 * - CLOCK_PROCESS_CPUTIME
 * - CLOCK_THREAD_CPUTIME
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL:  \a id is not one of the above clocks or tp is NULL
 *
 */
int clock_gettime(clockid_t id, struct timespec *tp) {
  u64 task_timer;
  struct mcu_timeval sched_time = {0};
  int pid;
  int i;

  if (tp == NULL) {
    errno = EINVAL;
    return -1;
  }

  switch (id) {
  case CLOCK_MONOTONIC:
  case CLOCK_REALTIME:
    cortexm_svcall(scheduler_timing_svcall_get_realtime, &sched_time);
    tp->tv_sec =
      sched_time.tv_sec * SCHEDULER_TIMEVAL_SECONDS + sched_time.tv_usec / 1000000UL;
    tp->tv_nsec = (sched_time.tv_usec % 1000000UL) * 1000UL;
    break;

  case CLOCK_PROCESS_CPUTIME_ID:
    // Sum the task timers for the calling process
    pid = task_get_pid(task_get_current());
    task_timer = 0;
    for (i = 0; i < task_get_total(); i++) {
      if (task_get_pid(i) == pid) {
        task_timer += task_gettime(i);
      }
    }
    task_timer_to_timespec(tp, task_timer);
    break;

  case CLOCK_THREAD_CPUTIME_ID:
    // The current task timer value
    //! \todo This should be an uninterruptable read
    task_timer = task_gettime(task_get_current());
    task_timer_to_timespec(tp, task_timer);
    break;

  default:
    errno = EINVAL;
    return -1;
  }
  return 0;
}

/*! \details This function gets the resolution of the \a id clock where \a id is one of:
 * - CLOCK_MONOTONIC
 * - CLOCK_REALTIME
 * - CLOCK_PROCESS_CPUTIME
 * - CLOCK_THREAD_CPUTIME
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - EINVAL:  \a id is not one of the above clocks or res is NULL
 *
 */
int clock_getres(clockid_t id, struct timespec *res) {

  if (res == NULL) {
    errno = EINVAL;
    return -1;
  }

  switch (id) {
  case CLOCK_MONOTONIC:
  case CLOCK_REALTIME:
    // 1 second
    res->tv_sec = 0;
    res->tv_nsec = 1000;
    break;

  case CLOCK_PROCESS_CPUTIME_ID:
  case CLOCK_THREAD_CPUTIME_ID:
    // One clock tick
    res->tv_sec = 0;
    res->tv_nsec = convert_clocks_to_nanoseconds(1);
    break;

  default:
    errno = EINVAL;
    return -1;
  }
  return 0;
}

/*! \details This function is not supported.
 *
 * \return Zero on success or -1 with errno (see \ref errno) set to:
 * - ENOTSUP:  not supported
 *
 */
int clock_settime(clockid_t id, const struct timespec *tp) {
  MCU_UNUSED_ARGUMENT(id);
  MCU_UNUSED_ARGUMENT(tp);
  errno = ENOTSUP;
  return -1;
}

/*! \cond */
int32_t convert_clocks_to_nanoseconds(int32_t clocks) {
  uint64_t tmp;
  tmp = (u64)clocks * SCHEDULER_CLOCK_NSEC_DIV + 512;
  return (u32)(tmp / 1024);
}

void task_timer_to_timespec(struct timespec *tp, u64 task_timer) {
  u64 nanosec;
  ldiv_t divide;
  divide = ldiv(task_timer, sos_config.sys.core_clock_frequency);
  nanosec = divide.rem * SCHEDULER_CLOCK_NSEC_DIV;
  tp->tv_sec = divide.quot;
  tp->tv_nsec = nanosec;
}
/*! \endcond */

/*! @} */
