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

#include "../scheduler/scheduler_local.h"

/*! \cond */
struct timeval time_of_day_offset MCU_SYS_MEM;
static void svcall_set_time(void *args) MCU_ROOT_EXEC_CODE;
void svcall_set_time(void *args) {
  CORTEXM_SVCALL_ENTER();
  div_t d;
  struct mcu_timeval tv;
  struct timeval tmp;
  struct timeval *t = (struct timeval *)args;
  scheduler_timing_svcall_get_realtime(&tv);

  // time = value + offset
  // offset = time - value
  d = div(tv.tv_usec, 1000000);
  tmp.tv_sec = tv.tv_sec * SCHEDULER_TIMEVAL_SECONDS + d.quot;
  tmp.tv_usec = d.rem;
  time_of_day_offset.tv_usec = t->tv_usec - tmp.tv_usec;
  time_of_day_offset.tv_sec = t->tv_sec - tmp.tv_sec;
  if (time_of_day_offset.tv_usec < 0) {
    time_of_day_offset.tv_usec += 1000000;
    time_of_day_offset.tv_sec--;
  }
}
/*! \endcond */

static int settimeofday_rtc(const struct timeval *tp);

int _settimeofday(const struct timeval *tp, const struct timezone *tzp) {
  MCU_UNUSED_ARGUMENT(tzp);
  settimeofday_rtc(tp);
  // also, set the simulated time
  cortexm_svcall(svcall_set_time, (void *)tp);

  return 0;
}

/*! \details This function sets the current time of day to the
 * time stored in \a tp.  The timezone (\a tzp) is ignored.
 *
 * \return Zero or -1 with errno (see \ref errno) set to:
 * - EIO: IO error when setting the real time clock
 *
 */
int settimeofday(const struct timeval *tp, const struct timezone *tzp){
  return _settimeofday(tp, tzp);
}

/*! \cond */



int settimeofday_rtc(const struct timeval *tp) {
  int fd;
  rtc_time_t cal_time;

  fd = open("/dev/rtc", O_RDWR);
  if (fd < 0) {
    return -1;
  }

  gmtime_r((time_t *)&tp, (struct tm *)&cal_time.time);
  cal_time.useconds = tp->tv_usec;

  if (ioctl(fd, I_RTC_SET, &cal_time) < 0) {
    close(fd);
    return -1;
  }

  return 0;
}
/*! \endcond */

/*! @} */
