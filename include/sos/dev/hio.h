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

/*! \addtogroup HIO Human Input/Output Device
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This device interface is for human interaction.  It is used
 * for devices such as buttons and LEDs.
 */

/*! \file  */
#ifndef SOS_DEV_HIO_H_
#define SOS_DEV_HIO_H_


#include <stdint.h>

#include "ioctl.h"
#include "mcu/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HIO_IOC_CHAR 'H'


/*! \brief HIO data types */
typedef enum {
	HIO_TYPE_BIT /*! \brief Unsigned 8-bit value */,
	HIO_TYPE_UINT8 /*! \brief Unsigned 8-bit value */,
	HIO_TYPE_UINT16 /*! \brief Unsigned 16-bit value */,
	HIO_TYPE_UINT32 /*! \brief Unsigned 32-bit value */,
	HIO_TYPE_UINT64 /*! \brief Unsigned 64-bit value */,
	HIO_TYPE_INT8 /*! \brief Signed 8-bit value */,
	HIO_TYPE_INT16 /*! \brief Signed 16-bit value */,
	HIO_TYPE_INT32 /*! \brief Signed 32-bit value */,
	HIO_TYPE_INT64 /*! \brief Signed 64-bit value */,
	HIO_TYPE_FLOAT /*! \brief Floating point value */,
	HIO_TYPE_DOUBLE /*! \brief Double Floating point value */
} hio_data_t;


/*! \brief HIO Input/Output types */
enum hio_io_type {
	HIO_EXTREMA /*! \brief Specifies the min/max of the previous type */,
	HIO_IN_BUTTON /*! \brief Momentary push button */,
	HIO_IN_DPAD /*! \brief Directional pad (up, down, left, right, center) */,
	HIO_IN_JOYSTICK /*! \brief Analog joystick */,
	HIO_IN_SWITCH /*! \brief Switch (stays in one position) */,
	HIO_IN_SELECTOR /*! \brief Selects one of several positions */,
	HIO_OUT_LED /*! \brief LED */,
	HIO_IN_STATUS /*! \brief Catch all status input */,
	HIO_OUT_STATUS /*! \brief Catch all status output */
};

/*! \brief IO subtypes */
enum hio_subtype {
	HIO_DPAD_UP /*! \brief D-pad up bit */ = (1<<0),
	HIO_DPAD_RIGHT /*! \brief D-pad right bit */  = (1<<1),
	HIO_DPAD_DOWN /*! \brief D-pad down bit */  = (1<<2),
	HIO_DPAD_LEFT /*! \brief D-pad left bit */  = (1<<3),
	HIO_DPAD_CENTER /*! \brief D-pad center bit */  = (1<<4),
	HIO_JOYSTICK_X /*! \brief Joystick X index */  = 0,
	HIO_JOYSTICK_Y /*! \brief Joystick Y index */  = 1,
	HIO_EXTREMA_MIN /*! \brief Extrema min index */  = 0,
	HIO_EXTREMA_MAX /*! \brief Extrema max index */  = 1
};

/*! \brief Descriptor entry
 * \details This data structure describes a single entry in the descriptor.
 *
 *
 * \code
 * const hio_desc_t desc[] = {
 * 	{ .type = HIO_TYPE_BIT, .use = HIO_IN_BUTTON, .count = 4 }, //four buttons (located in LSbs)
 * 	{ .type = HIO_TYPE_UINT8, .use = HIO_IN_DPAD, .count = 1 }, //directional pad
 * 	{ .type = HIO_TYPE_UINT8, .use = HIO_OUT_LED, .count = 1 }, //LED
 * 	{ .type = HIO_TYPE_INT16, .use = HIO_IN_JOYSTICK, .count = 2 }, //2-axis joystick x then y
 * 	{ .type = HIO_TYPE_INT16, .use = HIO_EXTREMA, .count = 2 }, //Min  then Max value for the joystick
 * 	};
 *
 *
 * 	typedef struct MCU_PACK {
 * 		u8 in_button; //four bits in LSb's
 * 		u8 in_dpad; //four bits HIO_DPAD_UP, etc
 * 		u8 out_led; //One bit in LSb
 * 		int16_t in_joystick[2]; //two analog values
 * 		int16_t joystick_extrema[2]; //min then max value of joystick
 * 	} my_desc_t;
 *
 * 	\endcode
 *
 *
 */
typedef struct MCU_PACK {
	u8 type /*! \brief The type of data (ie HIO_TYPE_UINT8) */;
	u8 use /*! \brief How the data is used (such as a button input, or LED) */;
	u16 count /*! \brief The number of values in the descriptor */;
} hio_desc_t;


/*! \brief HIO attributes
 * \details This contains the attributes of the human input/output device
 */
typedef struct MCU_PACK {
	const hio_desc_t * report_desc /*! \brief A pointer to the HIO report descriptor */;
	u16 count /*! \brief Number of total descriptor entries (hio_desc_t) */;
	u16 size /*! \brief Size in bytes of the report */;
} hio_attr_t;


/*! \details This request gets the attributes of the device.
 *
 * Example:
 * \code
 * hio_attr_t attr;
 * ioctl(fildes, I_HIO_GETINFO, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_HIO_GETINFO _IOCTLR(HIO_IOC_CHAR, 0, hio_attr_t)

#define I_HIO_SETATTR _IOCTLW(HIO_IOC_CHAR, 1, hio_attr_t)

/*! \details This request writes to outputs.  For example,
 * it can be used to turn an LED on and off if the
 * descriptor contains a HIO_OUT_LED type.
 *
 * \sa hio_desc_t
 *
 *
 */
#define I_HIO_SETREPORT _IOCTL(HIO_IOC_CHAR, 2)


/*! \brief See below for details
 * \details This request gets the status of the
 * report.  This can be used to read the value of the inputs (such
 * as a HIO_IN_BUTTON).
 *
 * The data format is according to the descriptor.
 *
 * \sa hio_desc_t
 *
 *
 *
 */
#define I_HIO_REPORT _IOCTL(HIO_IOC_CHAR, 3)
#define I_HIO_GETREPORT I_HIO_REPORT

/*! \brief Initialize the driver
 * \details This initializes the driver.  It is usually
 * executed by the system and doesn't need initialization by
 * any applications.
 *
 */
#define I_HIO_INIT _IOCTL(HIO_IOC_CHAR, 4)


#define I_HIO_TOTAL 5


#ifdef __cplusplus
}
#endif


#endif /* SOS_DEV_HIO_H_ */
