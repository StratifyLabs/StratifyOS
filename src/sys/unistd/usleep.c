// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include <errno.h>
#include <mcu/tmr.h>
#include <time.h>
#include <unistd.h>

#include "../scheduler/scheduler_timing.h"
#include "sos/debug.h"

/*! \cond */
static void svcall_usleep(void *args) MCU_ROOT_EXEC_CODE;
static void svcall_get_usecond_tmr(void *args);
/*! \endcond */


/*! \details Causes the calling thread to sleep for \a useconds microseconds.
 *
 * If useconds is greater than a threshold, the calling thread will
 * yield the processor.
 *
 * \return 0 or -1 for an error with errno (see \ref errno) set to:
 * - EINVAL: \a useconds is greater than 1 million.
 */
int usleep(useconds_t useconds) {
  SOS_DEBUG_ENTER_TIMER_SCOPE_AVERAGE(usleep);
  if (task_get_current()) {
    scheduler_check_cancellation();
  }

  if (useconds == 0) {
    return 0;
  }

  if (useconds <= 1000000UL) {
    const u32 clocks = scheduler_timing_useconds_to_clocks(useconds);
    const u32 tmp = scheduler_timing_useconds_to_clocks(1);
    if ((task_get_current() == 0) || (clocks < 8000)) {

      // Issue #61 -- read the microsecond timer so that the delay is more accurate
      u32 now;
      u32 start;
      u32 end;

      cortexm_svcall(svcall_get_usecond_tmr, &start);
      end = start + useconds;

      if (end > SOS_USECOND_PERIOD) {
        end -= SOS_USECOND_PERIOD; // adjust for overflow
      }
      // static wait loop
      do {
        cortexm_svcall(svcall_get_usecond_tmr, &now);
      } while ((now <= end) && (now >= start));

    } else {
      // clocks is greater than 8000 -- there is time to change to another task
      // use a fudge factor adjustment to account for clocks taken to start sleeping
      useconds -= (600 / tmp);
      cortexm_svcall(svcall_usleep, &useconds);
    }
  } else {
    errno = EINVAL;
    return -1;
  }

#if SOS_DEBUG
  usleep_sum -= useconds;
#endif
  SOS_DEBUG_EXIT_TIMER_SCOPE_AVERAGE(SOS_DEBUG_UNISTD, usleep, 100);
  return 0;
}

/*! \cond */
void svcall_get_usecond_tmr(void *args) {
  CORTEXM_SVCALL_ENTER();
  u32 *value = args;
  *value = sos_config.clock.microseconds();
}

void svcall_usleep(void *args) {
  CORTEXM_SVCALL_ENTER();
  const useconds_t *p;
  struct mcu_timeval abs_time;
  p = (useconds_t *)args;
  scheduler_timing_svcall_get_realtime(&abs_time);
  abs_time.tv_usec = abs_time.tv_usec + *p;

  if (abs_time.tv_usec > SOS_USECOND_PERIOD) {
    abs_time.tv_sec++;
    abs_time.tv_usec -= SOS_USECOND_PERIOD;
  }

  scheduler_timing_root_timedblock(NULL, &abs_time);
}
/*! \endcond */

/*! @} */
