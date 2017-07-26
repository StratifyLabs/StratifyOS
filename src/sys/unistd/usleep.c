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

/*! \addtogroup UNI_SLEEP
 * @{
 */

/*! \file */


#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "mcu/debug.h"
#include "../sched/sched_flags.h"

static void priv_usleep(void * args) MCU_PRIV_EXEC_CODE;

/*! \details This function causes the calling thread to sleep for \a useconds microseconds.
 *
 * \return 0 or -1 for an error with errno (see \ref ERRNO) set to:
 * - EINVAL: \a useconds is greater than 1 million.
 */
int usleep(useconds_t useconds){
	uint32_t clocks;
	uint32_t tmp;
	int i;
	if ( useconds == 0 ){
		return 0;
	}
	if ( useconds <= 1000000UL ){
		clocks =  sched_useconds_to_clocks(useconds);
		tmp = sched_useconds_to_clocks(1);
		if( (task_get_current() == 0) || (clocks < 8000) ){

			for(i = 0; i < clocks; i+=14){
				asm volatile("nop");
				asm volatile("nop");
				asm volatile("nop");
				asm volatile("nop");
				asm volatile("nop");
				asm volatile("nop");
				asm volatile("nop");
				asm volatile("nop");
				asm volatile("nop");
				asm volatile("nop");
			}

		} else {
			//clocks is greater than 4800 -- there is time to change to another task
			useconds -= (600 / tmp);
			cortexm_svcall(priv_usleep, &useconds);
		}
	} else {
		errno = EINVAL;
		return -1;
	}
	return 0;
}

void priv_usleep(void * args){
	useconds_t * p;
	struct sched_timeval abs_time;
	p = (useconds_t*)args;
	sched_priv_get_realtime(&abs_time);
	abs_time.tv_usec = abs_time.tv_usec + *p;

	if ( abs_time.tv_usec > STFY_USECOND_PERIOD ){
		abs_time.tv_sec++;
		abs_time.tv_usec -= STFY_USECOND_PERIOD;
	}

	sched_priv_timedblock(NULL, &abs_time);
}

/*! @} */
