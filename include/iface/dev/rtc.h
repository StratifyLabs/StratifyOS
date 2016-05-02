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

/*! \addtogroup RTC Real Time Clock (RTC)
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The Real Time Clock (RTC) module is an API to access an MCUs integrated real time clock.
 *
 * More information about accessing peripheral IO is in the \ref IFACE_DEV section.
 */

/*! \file
 * \brief Real Time Clock Header File
 *
 */

#ifndef DEV_RTC_H_
#define DEV_RTC_H_

#include <time.h>
#include <stdint.h>
#include "ioctl.h"
#include "mcu/arch.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RTC_IOC_IDENT_CHAR 'r'

/*! \details This defines a RTC action.
 *
 */
typedef mcu_action_t rtc_action_t;

/*! \brief See below for details.
 * \details These are the values for \a clock of \ref rtc_attr_t.
 */
typedef enum {
	RTC_CLKSRC_EXTERNAL_32768 /*! External 32.768KHz Crystal */,
	RTC_CLKSRC_INTERNAL_40000 /*! Internal 40KHz Oscillator */
} rtc_clock_t;

/*! \details This lists the bit assignments when
 * using the RTC interrupt callback.
 *
 */
typedef enum {
	RTC_EVENT_ALARM,
	RTC_EVENT_COUNT
} rtc_event_t;

/*! \details This lists the times when the count
 * interrupt can be set.
 */
typedef enum {
	RTC_EVENT_COUNT_NONE /*! Do not interrupt based on the count (default) */,
	RTC_EVENT_COUNT_SECOND /*! Interrupt every second */,
	RTC_EVENT_COUNT_MINUTE /*! Interrupt every minute */,
	RTC_EVENT_COUNT_HOUR /*! Interrupt every hour */,
	RTC_EVENT_COUNT_DAY /*! Interrupt every day */,
	RTC_EVENT_COUNT_WEEK /*! Interrupt every week */,
	RTC_EVENT_COUNT_MONTH /*! Interrupt every month */,
	RTC_EVENT_COUNT_YEAR /*! Interrupt every year */
} rtc_event_count_t;

/*!
 * \brief Lists the days of the week.
 * \details This is a list of the valid days of
 * the week.
 */
typedef enum {
	RTC_DAY_SUNDAY /*! Sunday */,
	RTC_DAY_MONDAY /*! Monday */,
	RTC_DAY_TUESDAY /*! Tuesday */,
	RTC_DAY_WEDNESDAY /*! Wednesday */,
	RTC_DAY_THURSDAY /*! Thursday */,
	RTC_DAY_FRIDAY /*! Friday */,
	RTC_DAY_SATURDAY /*! Saturday */
} rtc_day_of_week_t;

/*! \brief Lists the months of the year.
 * \details This is a list of the valid months of the year.
 */
typedef enum {
	RTC_MON_JAN = 0 /*! January */,
	RTC_MON_FEB /*! February */,
	RTC_MON_MAR /*! March */,
	RTC_MON_APR /*! April */,
	RTC_MON_MAY /*! May */,
	RTC_MON_JUN /*! June */,
	RTC_MON_JUL /*! July */,
	RTC_MON_AUG /*! August */,
	RTC_MON_SEP /*! September */,
	RTC_MON_OCT /*! October */,
	RTC_MON_NOV /*! November */,
	RTC_MON_DEC /*! December */
} rtc_mon_t;


/*! \brief RTC Time Structure (compatible with struct tm from time.h)
 * \details This structure holds the time.  It should be compatible
 * with struct tm.
 */
typedef struct MCU_PACK {
	struct link_tm time /*! \brief Time (compatible with struct tm) used with set/get */;
	uint32_t useconds /*! \brief Microseconds used with set/get */;
	uint32_t time_t;
	uint32_t use_time_t;
} rtc_time_t;


/*! \brief Lists the valid alarm types.
 * \details This enumerates the valid alarm types.
 */
typedef enum {
	RTC_ALARM_ONCE /*! One time alarm */,
	RTC_ALARM_MINUTE /*! Alarm every minute */,
	RTC_ALARM_HOURLY /*! Alarm every hour */,
	RTC_ALARM_DAILY /*! Daily alarm */,
	RTC_ALARM_WEEKLY /*! Weekly alarm */,
	RTC_ALARM_MONTHLY /*! Monthly alarm */,
	RTC_ALARM_YEARLY /*! Yearly alarm */
} rtc_alarm_type_t;

/*! \brief RTC Alarm including Time and Alarm Type
 * \details This structure is used
 * to read and write the RTC alarm
 * using the rtc_ioctl() function.
 *
 */
typedef struct MCU_PACK {
	rtc_time_t time /*! \brief The time for the alarm */;
	uint8_t type /*! \brief The type of alarm (\ref rtc_alarm_type_t) */;
} rtc_alarm_t;


/*! \brief RTC IO Attributes
 * \details This data structure defines
 * the structure used with I_GETATTR and I_SETATTR
 * ioctl requests on the RTC.  It is also used when opening the RTC.
 */
typedef struct MCU_PACK {
	uint8_t pin_assign /*! Always 0 */;
	uint8_t clock /*! The source clock */;
	uint32_t freq /*! Calibration frequency for setting the pre-scalar */;
} rtc_attr_t;

#define I_RTC_GETATTR _IOCTLR(RTC_IOC_IDENT_CHAR, I_GLOBAL_GETATTR, rtc_attr_t)
#define I_RTC_SETATTR _IOCTLW(RTC_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, rtc_attr_t)
#define I_RTC_SETACTION _IOCTLW(RTC_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, rtc_action_t)

/*! \brief See details below.
 * \details This request writes the alarm value.
 * The ctl argument is a pointer to an rtc_alarm_t
 * which contains the alarm.
 *
 * Example:
 * \code
 * rtc_alarm_t alarm;
 * //set alarm members
 * ioctl(rtc_fd, I_RTC_SETALARM, &alarm);
 * \endcode
 *
 * \hideinitializer
 */
#define I_RTC_SETALARM _IOCTLW(RTC_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 0, rtc_alarm_t)
#define I_RTC_SET_ALARM I_RTC_SETALARM

/*! \brief See details below.
 * \details This request reads the alarm value.
 * The ctl argument is a pointer to memory
 * where the rtc_alarm_t will be stored.
 *
 * Example:
 * \code
 * rtc_alarm_t alarm;
 * ioctl(rtc_fd, I_RTC_GETALARM, &alarm);
 * //alarm is populated with the current alarm values
 * \endcode
 * \hideinitializer
 */
#define I_RTC_GETALARM _IOCTLR(RTC_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 1, rtc_alarm_t)
#define I_RTC_GET_ALARM I_RTC_GETALARM


/*! \brief See details below.
 * \details This request disable the alarm.  The ctl
 * argument is NULL.
 *
 * Example:
 * \code
 * ioctl(rtc_fd, I_RTC_DISABLEALARM);
 * \endcode
 *
 * \hideinitializer
 */
#define I_RTC_DISABLEALARM _IOCTL(RTC_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 2)
#define I_RTC_DISABLE_ALARM I_RTC_DISABLEALARM

/*! \brief See details below.
 * \details This request writes the value of the timer.
 * For example:
 * \code
 * rtc_time_t the_time;
 * ioctl(rtc_fd, I_RTC_SET, &the_time);
 * \endcode
 * \hideinitializer
 */
#define I_RTC_SET _IOCTLW(RTC_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 3, rtc_time_t)

/*! \brief See details below.
 * \details This request reads the value of the timer.
 * For example:
 * \code
 * rtc_time_t current_time;
 * ioctl(rtc_fd, I_RTC_GET, &current_time );
 * \endcode
 * \hideinitializer
 */
#define I_RTC_GET _IOCTLR(RTC_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 4, rtc_time_t)


/*! \brief See details below.
 * \details This request controls when the count action
 * is triggered.  The ctl argument is a \ref rtc_event_count_t.
 *
 * \code
 * ioctl(rtc_fd, I_RTC_SETCOUNTEVENT, (void*)RTC_EVENT_COUNT_SECOND );
 * \endcode
 * \hideinitializer
 */
#define I_RTC_SETCOUNTEVENT _IOCTL(RTC_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 7)
#define I_RTC_SET_COUNT_EVENT I_RTC_SETCOUNTEVENT

#define I_RTC_TOTAL 8

#ifdef __cplusplus
}
#endif

#endif /* DEV_RTC_H_ */

/*! @} */
