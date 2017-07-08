/* Copyright 2011-2016 Tyler Gilbert; 
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

/*! \addtogroup TIME
 * @{
 */

/*! \file */

#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "mcu/mcu.h"
#include "sos/dev/rtc.h"

#include "../sched/sched_flags.h"

extern struct timeval time_of_day_offset MCU_SYS_MEM;

/*! \details This function gets the current time of day using
 * the real time clock.  The time is written to \a tp.  In
 * this implementation \a tzp is not used.
 *
 * \return Zero
 *
 */
int gettimeofday(struct timeval * tp, void * tzp);

static int gettimeofday_rtc(struct timeval * ptimeval);
static void gettimeofday_sched(struct timeval * ptimeval);

int _gettimeofday(struct timeval * ptimeval, void * ptimezone){
	if ( gettimeofday_rtc(ptimeval) < 0 ){
		gettimeofday_sched(ptimeval);
	}
	return 0;
}

void gettimeofday_sched(struct timeval * ptimeval){
	struct sched_timeval tv;
	div_t d;
	struct timeval tmp;
	mcu_core_privcall((core_privcall_t)sched_priv_get_realtime, &tv);

	//Convert the sched_timeval to a timeval struct
	d = div(tv.tv_usec, 1000000);
	tmp.tv_sec = tv.tv_sec * SCHED_TIMEVAL_SECONDS + d.quot;
	tmp.tv_usec = d.rem;
	//add the offset
	ptimeval->tv_sec = tmp.tv_sec + time_of_day_offset.tv_sec;
	ptimeval->tv_usec = tmp.tv_usec + time_of_day_offset.tv_usec;
	if ( ptimeval->tv_usec >= 1000000 ){
		ptimeval->tv_usec -= 1000000;
		ptimeval->tv_sec++;
	}
	return;
}

int gettimeofday_rtc(struct timeval * ptimeval){
	int fd;
	struct tm cal_time;

	fd = open("/dev/rtc", O_RDWR);
	if ( fd < 0 ){
		return -1;
	}

	if ( ioctl(fd, I_RTC_GET, &cal_time) < 0){
		close(fd);
		return -1;
	}

	ptimeval->tv_sec = mktime(&cal_time);
	ptimeval->tv_usec = 0;
	close(fd);
	return 0;
}

/*! @} */
