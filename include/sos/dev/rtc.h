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

#ifndef SOS_DEV_RTC_H_
#define SOS_DEV_RTC_H_

#include "mcu/types.h"
#include "sos/link/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTC_VERSION (0x030000)
#define RTC_IOC_IDENT_CHAR 'r'

/*! \details RTC flags used with
 * rtc_attr_t.o_flags and I_RTC_SETATTR.
 *
 */
typedef enum {
	RTC_FLAG_NONE /*! No Flags */ = 0,
	RTC_FLAG_ENABLE /*! Set the alarm */ = (1<<0),
	RTC_FLAG_DISABLE /*! Set the alarm */ = (1<<1),
	RTC_FLAG_IS_SOURCE_EXTERNAL_32768 /*! External 32.768KHz Crystal */ = (1<<2),
	RTC_FLAG_IS_SOURCE_INTERNAL_40000 /*! Internal 40KHz Oscillator */ = (1<<3),
	RTC_FLAG_ENABLE_ALARM /*! Enable the alarm */ = (1<<4),
	RTC_FLAG_DISABLE_ALARM /*! Enable the alarm */ = (1<<5),
	RTC_FLAG_IS_ALARM_ONCE /*! One time alarm */ = (1<<6),
	RTC_FLAG_IS_ALARM_MINUTE /*! Alarm every minute */ = (1<<7),
	RTC_FLAG_IS_ALARM_HOURLY /*! Alarm every hour */ = (1<<8),
	RTC_FLAG_IS_ALARM_DAILY /*! Daily alarm */ = (1<<9),
	RTC_FLAG_IS_ALARM_WEEKLY /*! Weekly alarm */ = (1<<10),
	RTC_FLAG_IS_ALARM_MONTHLY /*! Monthly alarm */ = (1<<11),
	RTC_FLAG_IS_ALARM_YEARLY /*! Yearly alarm */ = (1<<12),
	RTC_FLAG_ENABLE_COUNT_EVENT /*! Enable a count event */ = (1<<13),
	RTC_FLAG_IS_COUNT_SECOND /*! One time alarm */ = (1<<14),
	RTC_FLAG_IS_COUNT_MINUTE /*! One time alarm */ = (1<<15),
	RTC_FLAG_IS_COUNT_HOUR /*! One time alarm */ = (1<<16),
	RTC_FLAG_IS_COUNT_DAY_OF_WEEK /*! One time alarm */ = (1<<17),
	RTC_FLAG_IS_COUNT_DAY_OF_MONTH /*! One time alarm */ = (1<<18),
	RTC_FLAG_IS_COUNT_DAY_OF_YEAR /*! One time alarm */ = (1<<19),
	RTC_FLAG_IS_COUNT_WEEK /*! One time alarm */ = (1<<20),
	RTC_FLAG_IS_COUNT_MONTH /*! One time alarm */ = (1<<21),
	RTC_FLAG_IS_COUNT_YEAR /*! One time alarm */ = (1<<22),
	RTC_FLAG_DISABLE_COUNT_EVENT /*! Enable a count event */ = (1<<23)
} rtc_flag_t;


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
	u32 useconds /*! \brief Microseconds used with set/get */;
	u32 time_t /*! c time value  (seconds since epoch) */;
	u32 use_time_t;
} rtc_time_t;

typedef struct {
	u32 o_flags;
	u32 o_events;
	rtc_time_t alarm;
	u32 resd[8];
} rtc_info_t;


/*! \brief RTC Attributes
 * \details This data structure defines
 * the structure used with I_RTC_ATTR and I_RTC_SETATTR
 * ioctl requests on the RTC.  It is also used when opening the RTC.
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Bitmask for setting attributes */;
	u32 freq /*! Calibration frequency for setting the pre-scalar */;
	rtc_time_t time /*! Time structure */;
	u32 resd[8] /*! Reserved */;
} rtc_attr_t;

#define I_RTC_GETVERSION _IOCTL(RTC_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_RTC_GETINFO _IOCTLR(RTC_IOC_IDENT_CHAR, I_MCU_GETINFO, rtc_attr_t)
#define I_RTC_SETATTR _IOCTLW(RTC_IOC_IDENT_CHAR, I_MCU_SETATTR, rtc_attr_t)
#define I_RTC_SETACTION _IOCTLW(RTC_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

/*! \brief See details below.
 * \details This request writes the value of the timer.
 * For example:
 * \code
 * rtc_time_t the_time;
 * ioctl(rtc_fd, I_RTC_SET, &the_time);
 * \endcode
 * \hideinitializer
 */
#define I_RTC_SET _IOCTLW(RTC_IOC_IDENT_CHAR, I_MCU_TOTAL + 0, rtc_time_t)

/*! \brief See details below.
 * \details This request reads the value of the timer.
 * For example:
 * \code
 * rtc_time_t current_time;
 * ioctl(rtc_fd, I_RTC_GET, &current_time );
 * \endcode
 * \hideinitializer
 */
#define I_RTC_GET _IOCTLR(RTC_IOC_IDENT_CHAR, I_MCU_TOTAL + 1, rtc_time_t)


#define I_RTC_TOTAL 2

#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_RTC_H_ */

/*! @} */
