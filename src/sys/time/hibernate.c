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

/*! \addtogroup TIME
 * @{
 */

/*! \file */


#include "config.h"
#include <errno.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "mcu/arch.h"
#include "mcu/mcu.h"
#include "mcu/wdt.h"
#include "sos/dev/rtc.h"

#include "../scheduler/scheduler_local.h"

static int set_alarm(int seconds);
static void root_powerdown(void * args) MCU_ROOT_EXEC_CODE;
static void root_hibernate(void * args) MCU_ROOT_EXEC_CODE;

void root_powerdown(void * args){
	mcu_core_execsleep(0, (void*)CORE_DEEPSLEEP_STANDBY);
}

void root_prepare_hibernate(void * args){
	int * seconds = (int*)args;
	//set the WDT to reset in args seconds
	if( (sos_board_config.o_sys_flags & SYS_FLAG_IS_WDT_DISABLED) == 0 ){
		if( *seconds != 0 ){
			mcu_wdt_setinterval((*seconds+1)*1000);
		} else {
			//set WDT timer to the max
			mcu_wdt_setinterval(INT_MAX);
		}
	}

	//The WDT only runs in hibernate on certain clock sources
	mcu_wdt_root_reset(NULL);

	//elevate task prio of caller so that nothing executes until prio is restored
	task_root_set_current_priority(SCHED_HIGHEST_PRIORITY+1);
	mcu_core_prepare_deepsleep(CORE_DEEPSLEEP);
}

void root_post_hibernate(void * args){
	cortexm_disable_interrupts();
	mcu_wdt_root_reset(NULL);
	mcu_core_recover_deepsleep(CORE_DEEPSLEEP);

	//restore task prio
	task_root_set_current_priority( task_get_priority(task_get_current()) );

	if( (sos_board_config.o_sys_flags & SYS_FLAG_IS_WDT_DISABLED) == 0 ){
		//Set WDT to previous value (it only runs in deep sleep with certain clock sources)
		mcu_wdt_setinterval(SCHED_RR_DURATION * 10 * sos_board_config.task_total + 5);
	}

	cortexm_enable_interrupts();

	//check to see if any higher prio tasks are ready to execute since the prio dropped
	scheduler_root_update_on_stopped();
}

void root_hibernate(void * args){
	root_prepare_hibernate(args);

	mcu_core_execsleep(0, (void*)CORE_DEEPSLEEP);

	//reinitialize the Clocks
	root_post_hibernate(args);
}

int set_alarm(int seconds){
	int fd;
	rtc_attr_t attr;
	int ret;

	fd = open("/dev/rtc", O_RDWR);
	if ( fd >= 0 ){
		attr.time.time_t = time(NULL);
		attr.time.time_t += seconds;
		gmtime_r((time_t*)&attr.time.time_t, (struct tm*)&attr.time.time);
		attr.o_flags = RTC_FLAG_IS_ALARM_ONCE | RTC_FLAG_ENABLE_ALARM;

		//set the alarm for "seconds" from now
		ret = ioctl(fd, I_RTC_SETATTR, &attr);
		close(fd);
		return ret;
	}
	return -1;
}

int hibernate(int seconds){
	if ( seconds > 0 ){
		set_alarm(seconds);
	}
	mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_HIBERNATE, &seconds);
	cortexm_svcall(root_hibernate, &seconds);
	mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_WAKEUP_FROM_HIBERNATE, &seconds);
	return 0;
}


void powerdown(int seconds){
	if ( seconds > 0 ){
		set_alarm(seconds);
	}
	mcu_board_execute_event_handler(MCU_BOARD_CONFIG_EVENT_POWERDOWN, &seconds);
	cortexm_svcall(root_powerdown, NULL);
	//device will reset after a powerdown event
}

/*! @} */
