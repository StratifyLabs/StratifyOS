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

/*! \addtogroup BUTTON_DEV Button Driver
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The Button driver interface defines
 * a way to read the status of buttons in a system.
 *
 * Buttons are binary on/off inputs. The button
 * is a block device where each location
 * represents different button. The status
 * of all the buttons can be read in a single command.
 *
 */

/*! \file
 * \brief Button Driver Header File
 *
 */

#ifndef SOS_DEV_BUTTON_H_
#define SOS_DEV_BUTTON_H_


#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BUTTON_VERSION (0x030000)


#define BUTTON_IOC_IDENT_CHAR 'B'

enum {
	BUTTON_NONE /*! No Button */,
	BUTTON_UP /*! Up Button */,
	BUTTON_DOWN /*! Down Button */,
	BUTTON_LEFT /*! LeftButton */,
	BUTTON_RIGHT /*! Right Button */,
	BUTTON_SELECT /*! Select Button */,
	BUTTON_BACK /*! Back Button */,
	BUTTON_EXIT /*! Exit Button */,
	BUTTON_USER0 /*! User button 0 */,
	BUTTON_USER1 /*! User button 1 */,
	BUTTON_USER2 /*! User button 2 */,
	BUTTON_USER3 /*! User button 3 */,
	BUTTON_USER4 /*! User button 4 */,
	BUTTON_USER5 /*! User button 5 */,
	BUTTON_USER6 /*! User button 6 */,
	BUTTON_USER7 /*! User button 7 */,
	BUTTON_USER8 /*! User button 8 */,
	BUTTON_USER9 /*! User button 9 */,
	BUTTON_USER10 /*! User button 10 */,
	BUTTON_USER11 /*! User button 11 */,
	BUTTON_USER12 /*! User button 12 */,
	BUTTON_USER13 /*! User button 13 */,
	BUTTON_USER14 /*! User button 14 */,
	BUTTON_USER15 /*! User button 15 */,
};

/*! \brief Button Flags for button_info_t and
 * button_status_t.
 *
 */
enum {
	BUTTON_FLAG_IS_PRESSED /*! Set if button was activated (set only once per button press) */ = (1<<0),
	BUTTON_FLAG_IS_RELEASED /*! Set if the button was released (set only once per button press) */ = (1<<1),
	BUTTON_FLAG_IS_ACTUATED /*! Set if the button was actuated (set only once per button press) */ = (1<<2),
	BUTTON_FLAG_IS_HELD /*! Set if the button was held for more than the specified threshold (set only once per button press) */ = (1<<3),
	BUTTON_FLAG_IS_ACTIVE /*! Set if the button is active when read */ = (1<<4),
	BUTTON_FLAG_IS_ACTIVE_HIGH /*! Set if the button is active high */ = (1<<5),
	BUTTON_FLAG_SET_ID /*! Sets the button ID (used in button_attr_t only) */ = (1<<6),
	BUTTON_FLAG_INITIALIZE /*! Initialize the button driver (used in button_attr_t only) */ = (1<<7),
	BUTTON_FLAG_INIT = BUTTON_FLAG_INITIALIZE,
	BUTTON_FLAG_DEINITIALIZE /*! Deinitialize the driver (used in button_attr_t only) */ = (1<<8),
	BUTTON_FLAG_DEINIT = BUTTON_FLAG_DEINITIALIZE,
	BUTTON_FLAG_SET_HELD_THRESHOLD /*! Sets the threshold in microseconds to trigger a button held event */ = (1<<9),
	BUTTON_FLAG_SET_ACTUATED_THRESHOLD /*! Sets the threshold in microseconds to trigger a button held event */ = (1<<10)
};

/*! \brief Button Read Data Structure
 * \details This data structure is used when
 * reading the buttons.
 *
 * \code
 * button_status_t status;
 * int fd = open("/dev/button0", O_RDWR);
 * lseek(fd, 0, SEEK_SET);
 * while( read(fd, &status, sizeof(status)) == sizeof(status) ){
 *   printf("Button is active? %d\n", (status.o_flags & BUTTON_FLAG_IS_ACTIVE) != 0);
 * }
 *
 * close(fd);
 *
 * \endcode
 *
 *
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Button Flags e.g., BUTTON_FLAG_IS_PRESSED */;
	u32 active_duration_msec /*! Number of milliseconds the button has been active */;
	u16 id /*! Button ID assigned using set attributes */;
	u16 resd; //align to 4 bytes
} button_status_t;

/*! \brief Button Info.
 * \details This structure contains the info for the
 * button driver.
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Set if the flag is supported in this driver */;
} button_info_t;

/*! \brief Button Attributes
 * \details This structure contains data used with
 * I_BUTTON_SETATTR.
 *
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Flags to apply to attributes setting e.g.BUTTON_FLAG_SET_ID */;
	u32 threshold_msec /*! Used when setting the button held or actuated thresholds */;
	u16 loc /*! The applicable location when setting the ID or a threshold (same as lseek()) */;
	u16 id /*! Sets the button ID when BUTTON_FLAG_SET_ID is set in o_flags */;
} button_attr_t;

#define I_BUTTON_GETVERSION _IOCTL(BUTTON_IOC_IDENT_CHAR, I_MCU_GETVERSION)

/*! \brief IOCTL request to get info.
 */
#define I_BUTTON_GETINFO _IOCTLR(BUTTON_IOC_IDENT_CHAR, I_MCU_GETINFO, button_info_t)

/*! \brief IOCTL request to set the attributes (set pin assignment)
 */
#define I_BUTTON_SETATTR _IOCTLW(BUTTON_IOC_IDENT_CHAR, I_MCU_SETATTR, button_attr_t)


#ifdef __cplusplus
}
#endif


#endif // SOS_DEV_BUTTON_H_

/*! @} */

