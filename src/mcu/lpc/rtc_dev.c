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
#include <errno.h>
#include "cortexm/cortexm.h"
#include "mcu/rtc.h"
#include "mcu/debug.h"
#include "mcu/core.h"

#define CLKEN (1<<0)
#define CTCRST (1<<1)
#define CCALEN (1<<4)

#define RTCCIF (1<<0)
#define RTCALF (1<<1)

#define AMRSEC (1<<0)
#define AMRMIN (1<<1)
#define AMRHOUR (1<<2)
#define AMRDOM (1<<3)
#define AMRDOW (1<<4)
#define AMRDOY (1<<5)
#define AMRMON (1<<6)
#define AMRYEAR (1<<7)

#if MCU_RTC_PORTS > 0

#if MCU_RTC_API == 1

#else
#define RTC_ISDST_REG (regs->GPREG0)
#endif

static LPC_RTC_Type * const rtc_regs[MCU_RTC_PORTS] = MCU_RTC_REGS;
static const u8 const rtc_irqs[MCU_RTC_PORTS] = MCU_RTC_IRQS;

typedef struct {
	mcu_event_handler_t handler;
	uint8_t ref_count;
} rtc_local_t;
static rtc_local_t rtc_local MCU_SYS_MEM;


static int set_alarm(int port, const rtc_attr_t * attr);
static int get_alarm(int port, rtc_info_t * info);
static int set_count_event(int port, u32 o_flags);

void mcu_rtc_dev_power_on(const devfs_handle_t * handle){
	int port = handle->port;
	LPC_RTC_Type * regs = rtc_regs[port];
	if ( rtc_local.ref_count == 0 ){
		rtc_local.handler.callback = NULL;
		regs->CCR = (CLKEN);
	}
	rtc_local.ref_count++;
}

void mcu_rtc_dev_power_off(const devfs_handle_t * handle){
	//LPC_RTC_Type * regs = rtc_regs[port];
	if ( rtc_local.ref_count > 0 ){
		if ( rtc_local.ref_count == 1 ){
			//core_priv_disable_irq((void*)RTC_IRQn);
			//regs->CCR = 0;
		}
		rtc_local.ref_count--;
	}
}

int mcu_rtc_dev_is_powered(const devfs_handle_t * handle){
	int port = handle->port;
	LPC_RTC_Type * regs = rtc_regs[port];

	if ( regs->CCR & (CLKEN) ){
		return 1;
	} else {
		return 0;
	}
}


int mcu_rtc_getinfo(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	rtc_info_t * info = ctl;
	info->o_flags = 0;
	info->o_events = 0;
	get_alarm(port, info);
	return 0;
}

int mcu_rtc_setattr(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	rtc_attr_t * attr = ctl;
	u32 o_flags = attr->o_flags;

	if( o_flags & RTC_FLAG_ENABLE ){

		if( o_flags & RTC_FLAG_IS_SOURCE_EXTERNAL_32768 ){

		}

	} else if( o_flags & RTC_FLAG_DISABLE ){

	}

	if( set_alarm(port, attr) < 0 ){
		return -1;
	}

	if( set_count_event(port, o_flags) < 0 ){
		return -1;
	}

	return 0;
}

int mcu_rtc_setaction(const devfs_handle_t * handle, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	int port = handle->port;
	if( cortexm_validate_callback(action->handler.callback) < 0 ){
		return -1;
	}

	rtc_local.handler = action->handler;

	cortexm_set_irq_prio(rtc_irqs[port], action->prio);
	return 0;
}

int mcu_rtc_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	errno = ENOTSUP;
	return -1;
}

int set_alarm(int port, const rtc_attr_t * attr){
	LPC_RTC_Type * regs = rtc_regs[port];
	u32 o_flags = attr->o_flags;

	if( o_flags & RTC_FLAG_ENABLE_ALARM ){

		cortexm_enable_irq((void*)RTC_IRQn);

		//elevate prio to come out of hibernate
		cortexm_set_irq_prio(RTC_IRQn, 3);


		regs->ASEC = attr->time.time.tm_sec;
		regs->AMIN = attr->time.time.tm_min;
		regs->AHRS = attr->time.time.tm_hour;
		regs->ADOM = attr->time.time.tm_mday;
		regs->ADOW = attr->time.time.tm_wday;
		regs->ADOY = attr->time.time.tm_yday + 1;
		regs->AMON = attr->time.time.tm_mon + 1;
		regs->AYRS = attr->time.time.tm_year;

		if( o_flags & RTC_FLAG_IS_ALARM_ONCE ){
			regs->AMR = AMRDOY|AMRDOW; //don't compare the day of the year or day of week
		} else if( o_flags & RTC_FLAG_IS_ALARM_MINUTE ){
			regs->AMR = (~(AMRSEC)) & 0xFF; //only compare seconds
		} else if( o_flags & RTC_FLAG_IS_ALARM_HOURLY ){
			regs->AMR = (~(AMRSEC|AMRMIN))  & 0xFF;
		} else if( o_flags & RTC_FLAG_IS_ALARM_DAILY ){
			regs->AMR =  (~(AMRSEC|AMRMIN|AMRHOUR))  & 0xFF;
		} else if( o_flags & RTC_FLAG_IS_ALARM_WEEKLY ){
			regs->AMR = (~(AMRSEC|AMRMIN|AMRHOUR|AMRDOW)) & 0xFF;
		} else if( o_flags & RTC_FLAG_IS_ALARM_MONTHLY ){
			regs->AMR = (~(AMRSEC|AMRMIN|AMRHOUR|AMRDOM)) & 0xFF;
		} else if( o_flags & RTC_FLAG_IS_ALARM_YEARLY ){
			regs->AMR = (~(AMRSEC|AMRMIN|AMRHOUR|AMRDOM|AMRMON)) & 0xFF;
		} else {

		}


	} else if( o_flags & RTC_FLAG_DISABLE_ALARM ){

	}
	return 0;
}

int get_alarm(int port, rtc_info_t * info){
	LPC_RTC_Type * regs = rtc_regs[port];
	info->alarm.time.tm_sec = regs->ASEC;
	info->alarm.time.tm_min = regs->AMIN;
	info->alarm.time.tm_hour = regs->AHRS;
	info->alarm.time.tm_mday = regs->ADOM;
	info->alarm.time.tm_wday = regs->ADOW;
	info->alarm.time.tm_yday = regs->ADOY - 1;
	info->alarm.time.tm_mon = regs->AMON - 1;
	info->alarm.time.tm_year = regs->AYRS;
	return 0;
}

int mcu_rtc_disablealarm(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_RTC_Type * regs = rtc_regs[port];

	regs->AMR = 0xFF;
	return 0;
}

int mcu_rtc_set(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_RTC_Type * regs = rtc_regs[port];

	rtc_time_t * timep;
	timep = (rtc_time_t*)ctl;
	regs->SEC = timep->time.tm_sec;
	regs->MIN = timep->time.tm_min;
	regs->HRS = timep->time.tm_hour;
	regs->DOW = timep->time.tm_wday;
	regs->DOM = timep->time.tm_mday;
	regs->DOY = timep->time.tm_yday + 1;
	regs->MONTH = timep->time.tm_mon + 1;
	regs->YEAR = timep->time.tm_year;
#if defined RTC_ISDST_REG
	RTC_ISDST_REG = timep->time.tm_isdst;
#endif
	return 0;
}

int mcu_rtc_get(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_RTC_Type * regs = rtc_regs[port];

	rtc_time_t * timep;
	timep = (rtc_time_t*)ctl;
	timep->time.tm_sec = regs->SEC;
	timep->time.tm_min = regs->MIN;
	timep->time.tm_hour = regs->HRS;
	timep->time.tm_wday = regs->DOW;
	timep->time.tm_mday = regs->DOM;
	timep->time.tm_yday = regs->DOY - 1;
	timep->time.tm_mon = regs->MONTH - 1;
	timep->time.tm_year = regs->YEAR;
#if defined RTC_ISDST_REG
	timep->time.tm_isdst = RTC_ISDST_REG;
#endif
	timep->useconds = 0;
	return 0;
}

int set_count_event(int port, u32 o_flags){
	LPC_RTC_Type * regs = rtc_regs[port];

	cortexm_enable_irq((void*)RTC_IRQn);

	if( o_flags & RTC_FLAG_DISABLE_COUNT_EVENT ){
		regs->CIIR = 0;
	} else if( o_flags & RTC_FLAG_ENABLE_COUNT_EVENT ){
		if( o_flags & RTC_FLAG_IS_COUNT_SECOND ){
			regs->CIIR |= (1<<0);
		}

		if( o_flags & RTC_FLAG_IS_COUNT_MINUTE ){
			regs->CIIR |= (1<<1);
		}

		if( o_flags & RTC_FLAG_IS_COUNT_HOUR ){
			regs->CIIR |= (1<<2);
		}

		if( o_flags & RTC_FLAG_IS_COUNT_DAY_OF_MONTH ){
			regs->CIIR |= (1<<3);
		}

		if( o_flags & RTC_FLAG_IS_COUNT_DAY_OF_WEEK ){
			regs->CIIR |= (1<<4);
		}

		if( o_flags & RTC_FLAG_IS_COUNT_DAY_OF_YEAR ){
			regs->CIIR |= (1<<5);
		}

		if( o_flags & RTC_FLAG_IS_COUNT_WEEK ){
			regs->CIIR |= (1<<4);
		}

		if( o_flags & RTC_FLAG_IS_COUNT_MONTH ){
			regs->CIIR |= (1<<6);
		}

		if( o_flags & RTC_FLAG_IS_COUNT_YEAR ){
			regs->CIIR |= (1<<7);
		}
	}
	return 0;
}

void mcu_core_rtc0_isr(){
	const int port = 0;
	LPC_RTC_Type * regs = rtc_regs[port];
	u32 o_events;
	devfs_handle_t handle;
	handle.port = port;
	rtc_event_t event;
	int flags;
	flags = regs->ILR & 0x03;
	regs->ILR = flags; //clear the flags
	o_events = 0;
	if ( flags & RTCALF ){
		o_events |= MCU_EVENT_FLAG_ALARM;
	}
	if ( flags & RTCCIF ){
		o_events |= MCU_EVENT_FLAG_COUNT;
	}

	mcu_rtc_get(&handle, &event.time);
	mcu_execute_event_handler(&(rtc_local.handler), o_events, &event);
}

#endif




