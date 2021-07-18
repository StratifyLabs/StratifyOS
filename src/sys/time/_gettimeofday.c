// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


/*! \addtogroup time
 * @{
 */

/*! \file */

#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "sos/dev/rtc.h"

#include "../scheduler/scheduler_timing.h"

extern struct timeval time_of_day_offset MCU_SYS_MEM;

/*! \details This function gets the current time of day using
 * the real time clock.  The time is written to \a tp.  In
 * this implementation \a tzp is not used.
 *
 * \return Zero
 *
 */
int gettimeofday(struct timeval *tp, void *tzp);

/*! \cond */
static int gettimeofday_rtc(struct timeval *ptimeval);
static void gettimeofday_sched(struct timeval *ptimeval);

int _gettimeofday(struct timeval *ptimeval, void *ptimezone) {
  if (gettimeofday_rtc(ptimeval) < 0) {
    gettimeofday_sched(ptimeval);
  }
  return 0;
}

void gettimeofday_sched(struct timeval *ptimeval) {
  struct mcu_timeval tv;
  div_t d;
  struct timeval tmp;
  cortexm_svcall((cortexm_svcall_t)scheduler_timing_svcall_get_realtime, &tv);

  // Convert the mcu_timeval to a timeval struct
  d = div(tv.tv_usec, 1000000);
  tmp.tv_sec = tv.tv_sec * SCHEDULER_TIMEVAL_SECONDS + d.quot;
  tmp.tv_usec = d.rem;
  // add the offset
  ptimeval->tv_sec = tmp.tv_sec + time_of_day_offset.tv_sec;
  ptimeval->tv_usec = tmp.tv_usec + time_of_day_offset.tv_usec;
  if (ptimeval->tv_usec >= 1000000) {
    ptimeval->tv_usec -= 1000000;
    ptimeval->tv_sec++;
  }
  return;
}

int gettimeofday_rtc(struct timeval *ptimeval) {
  int fd;
  struct tm cal_time;

  fd = open("/dev/rtc", O_RDWR);
  if (fd < 0) {
    return -1;
  }

  ioctl(fd, I_RTC_GET, &cal_time);
  ptimeval->tv_sec = mktime(&cal_time);
  ptimeval->tv_usec = 0;
  close(fd);
  return 0;
}
/*! \endcond */

/*! @} */
