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

/*! \addtogroup unistd
 * @{
 */

/*! \file */

#include <unistd.h>
#include <time.h>
#include "../scheduler/scheduler_local.h"

/*! \cond */
static void root_sleep(void * args) MCU_ROOT_EXEC_CODE;
/*! \endcond */

/*! \details This function causes the calling thread to sleep for \a seconds seconds.
 * \return 0
 */
unsigned int sleep(unsigned int seconds /*! The number of seconds to sleep */){
	struct mcu_timeval interval;
	div_t d;
	if ( task_get_current() != 0 ){
		if ( seconds < SCHEDULER_TIMEVAL_SECONDS ){
			interval.tv_sec = 0;
			interval.tv_usec = seconds * 1000000;
		} else {
			d = div(seconds, SCHEDULER_TIMEVAL_SECONDS);
			interval.tv_sec = d.quot;
			interval.tv_usec = d.rem;
		}
		cortexm_svcall(root_sleep, &interval);
	}
	return 0;
}

/*! \cond */
void root_sleep(void * args){
	struct mcu_timeval * p;
	struct mcu_timeval abs_time;
	p = (struct mcu_timeval*)args;
	scheduler_timing_root_get_realtime(&abs_time);
	abs_time.tv_sec += p->tv_sec;
	abs_time.tv_usec += p->tv_usec;
	if ( abs_time.tv_usec > SCHEDULER_TIMEVAL_SECONDS*1000000 ){
		abs_time.tv_sec++;
		abs_time.tv_usec -= SCHEDULER_TIMEVAL_SECONDS*1000000;
	}
	scheduler_timing_root_timedblock(NULL, &abs_time);
}
/*! \endcond */

/*! @} */
