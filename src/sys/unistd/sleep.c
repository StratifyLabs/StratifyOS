// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include "../scheduler/scheduler_local.h"
#include <time.h>
#include <unistd.h>

/*! \cond */
static void svcall_sleep(void *args) MCU_ROOT_EXEC_CODE;
/*! \endcond */

/*! \details This function causes the calling thread to sleep for \a seconds seconds.
 * \return 0
 */
unsigned int sleep(unsigned int seconds /*! The number of seconds to sleep */) {
  struct mcu_timeval interval;
  if (task_get_current() != 0) {
    scheduler_check_cancellation();
    if (seconds < SCHEDULER_TIMEVAL_SECONDS) {
      interval.tv_sec = 0;
      interval.tv_usec = seconds * 1000000;
    } else {
      div_t d = div(seconds, SCHEDULER_TIMEVAL_SECONDS);
      interval.tv_sec = d.quot;
      interval.tv_usec = d.rem;
    }
    cortexm_svcall(svcall_sleep, &interval);
  }
  return 0;
}

/*! \cond */
void svcall_sleep(void *args) {
  CORTEXM_SVCALL_ENTER();
  struct mcu_timeval *p;
  struct mcu_timeval abs_time;
  p = (struct mcu_timeval *)args;
  scheduler_timing_svcall_get_realtime(&abs_time);
  abs_time.tv_sec += p->tv_sec;
  abs_time.tv_usec += p->tv_usec;
  if (abs_time.tv_usec > SCHEDULER_TIMEVAL_SECONDS * 1000000) {
    abs_time.tv_sec++;
    abs_time.tv_usec -= SCHEDULER_TIMEVAL_SECONDS * 1000000;
  }
  scheduler_timing_root_timedblock(NULL, &abs_time);
}
/*! \endcond */

/*! @} */
