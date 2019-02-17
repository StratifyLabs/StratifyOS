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


#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "mcu/mcu.h"
#include "sos/dev/rtc.h"

#include "../scheduler/scheduler_local.h"

/*! \cond */
struct timeval time_of_day_offset MCU_SYS_MEM;
static void root_set_time(void * args) MCU_ROOT_EXEC_CODE;
void root_set_time(void * args){
	div_t d;
	struct mcu_timeval tv;
	struct timeval tmp;
	struct timeval * t = (struct timeval *)args;
	scheduler_timing_root_get_realtime(&tv);

	//time = value + offset
	//offset = time - value
	d = div(tv.tv_usec, 1000000);
	tmp.tv_sec = tv.tv_sec * SCHEDULER_TIMEVAL_SECONDS + d.quot;
	tmp.tv_usec = d.rem;
	time_of_day_offset.tv_usec = t->tv_usec - tmp.tv_usec;
	time_of_day_offset.tv_sec = t->tv_sec - tmp.tv_sec;
	if( time_of_day_offset.tv_usec < 0 ){
		time_of_day_offset.tv_usec += 1000000;
		time_of_day_offset.tv_sec--;
	}
}
/*! \endcond */

/*! \details This function sets the current time of day to the
 * time stored in \a tp.  The timezone (\a tzp) is ignored.
 *
 * \return Zero or -1 with errno (see \ref errno) set to:
 * - EIO: IO error when setting the real time clock
 *
 */
int settimeofday(const struct timeval * tp, const struct timezone * tzp);

/*! \cond */
static int settimeofday_rtc(const struct timeval * tp);

int _settimeofday(const struct timeval * tp, const struct timezone * tzp) {
	settimeofday_rtc(tp);
	//also, set the simulated time
	cortexm_svcall(root_set_time, (void*)tp);

	return 0;
}

int settimeofday_rtc(const struct timeval * tp){
	int fd;
	rtc_time_t cal_time;

	fd = open("/dev/rtc", O_RDWR);
	if ( fd < 0 ){
		return -1;
	}

	gmtime_r( &tp->tv_sec, (struct tm*)&cal_time.time);
	cal_time.useconds = tp->tv_usec;

	if (ioctl(fd, I_RTC_SET, &cal_time) < 0 ){
		close(fd);
		return -1;
	}

	return 0;
}
/*! \endcond */


/*! @} */
