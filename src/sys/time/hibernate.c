/* Copyright 2011-2018 Tyler Gilbert;
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

/*! \addtogroup time
 * @{
 */

/*! \file */

#include "config.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "mcu/arch.h"
#include "mcu/mcu.h"
#include "mcu/wdt.h"
#include "sos/dev/rtc.h"

#include "../scheduler/scheduler_local.h"

/*! \cond */
static int set_alarm(int seconds);
static void svcall_powerdown(void *args) MCU_ROOT_EXEC_CODE;
static void svcall_hibernate(void *args) MCU_ROOT_EXEC_CODE;

void svcall_powerdown(void *args) {
  CORTEXM_SVCALL_ENTER();
  if (sos_config.sleep.powerdown) {
    sos_config.sleep.powerdown();
  }
}

void root_prepare_hibernate(void *args) {
  // elevate task prio of caller so that nothing executes until prio is restored
  task_root_set_current_priority(SCHED_HIGHEST_PRIORITY + 1);
  sos_handle_event(SOS_EVENT_ROOT_PREPARE_DEEPSLEEP, 0);
}

void root_post_hibernate(void *args) {
  cortexm_disable_interrupts();
  sos_handle_event(SOS_EVENT_ROOT_RECOVER_DEEPSLEEP, 0);

  // restore task prio
  task_root_set_current_priority(task_get_priority(task_get_current()));

  cortexm_enable_interrupts();

  // check to see if any higher prio tasks are ready to execute since the prio dropped
  scheduler_root_update_on_stopped();
}

void svcall_hibernate(void *args) {
  CORTEXM_SVCALL_ENTER();
  root_prepare_hibernate(args);

  if (sos_config.sleep.hibernate) {
    int *seconds = args;
    sos_config.sleep.hibernate(*seconds);
  }

  // reinitialize the Clocks
  root_post_hibernate(args);
}

int set_alarm(int seconds) {
  int fd;
  rtc_attr_t attr;

  fd = open("/dev/rtc", O_RDWR);
  if (fd >= 0) {
    attr.time.time_t = time(NULL);
    attr.time.time_t += seconds;
    gmtime_r((time_t *)&attr.time.time_t, (struct tm *)&attr.time.time);
    attr.o_flags = RTC_FLAG_IS_ALARM_ONCE | RTC_FLAG_ENABLE_ALARM;

    // set the alarm for "seconds" from now
    int ret = ioctl(fd, I_RTC_SETATTR, &attr);
    close(fd);
    return ret;
  }
  return -1;
}

int hibernate(int seconds) {
  if (seconds > 0) {
    set_alarm(seconds);
  }
  sos_handle_event(SOS_EVENT_HIBERNATE, &seconds);
  cortexm_svcall(svcall_hibernate, &seconds);
  sos_handle_event(SOS_EVENT_WAKEUP_FROM_HIBERNATE, &seconds);
  return 0;
}

void powerdown(int seconds) {
  if (seconds > 0) {
    set_alarm(seconds);
  }
  sos_handle_event(SOS_EVENT_POWERDOWN, &seconds);
  cortexm_svcall(svcall_powerdown, NULL);
  // device will reset after a powerdown event
}
/*! \endcond */

/*! @} */
