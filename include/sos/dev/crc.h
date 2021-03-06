// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup CRC Cyclic Redundancy Check (CRC)
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The Cyclic Redundancy Check (CRC) module is an API to access an MCUs integrated cyclic redundancy
 * check hardware unit.
 *
 * More information about accessing peripheral IO is in the \ref IFACE_DEV section.
 */

/*! \file
 * \brief Cyclic Redundancy Check Header File
 *
 */

#ifndef SOS_DEV_CRC_H_
#define SOS_DEV_CRC_H_

#include <sdk/types.h>
#include "sos/link/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CRC_VERSION (0x030000)
#define CRC_IOC_IDENT_CHAR 'C'

/*! \details CRC flags used with
 * crc_attr_t.o_flags and I_CRC_SETATTR.
 *
 */
typedef enum {
	CRC_FLAG_NONE /*! No Flags */ = 0,
	CRC_FLAG_ENABLE /*! Enable the CRC */ = (1<<0),
	CRC_FLAG_DISABLE /*! Disable the CRC */ = (1<<1),
	CRC_FLAG_IS_32BIT /*! Use a 32-bit CRC */ = (1<<2),
	CRC_FLAG_IS_16BIT /*! Use a 16-bit CRC */ = (1<<3),
	CRC_FLAG_IS_8BIT /*! Use a 8-bit CRC */ = (1<<4),
	CRC_FLAG_IS_7BIT /*! Use a 7-bit CRC */ = (1<<5),
	CRC_FLAG_IS_INVERT_OUTPUT /*! Invert the output */ = (1<<6),
	CRC_FLAG_IS_DEFAULT_POLYNOMIAL /*! Use the default polynomial */ = (1<<7),
	CRC_FLAG_IS_DEFAULT_INTIAL_VALUE /*! Use the default initial value */ = (1<<8),
	CRC_FLAG_IS_INVERT_INPUT_8BIT /*! Invert the input as bytes */ = (1<<9),
	CRC_FLAG_IS_INVERT_INPUT_16BIT /*! Invert the input as 16-bit words */ = (1<<10),
	CRC_FLAG_IS_INVERT_INPUT_32BIT /*! Invert the input as 32-bit words */ = (1<<11),
	CRC_FLAG_IS_INPUT_32BIT /*! Input data is 32 bit words */ = (1<<12),
	CRC_FLAG_IS_INPUT_16BIT /*! Input data is 16 bit words */ = (1<<13),
	CRC_FLAG_IS_INPUT_8BIT /*! Input data is bytes */ = (1<<14),
} crc_flag_t;


typedef struct {
	u32 o_flags;
	u32 o_events;
	u32 polynomial;
	u32 initial_value;
	u32 resd[8];
} crc_info_t;


/*! \brief CRC Attributes
 * \details This data structure defines
 * the structure used with I_CRC_SETATTR
 * ioctl requests on the RTC.  It is also used when opening the RTC.
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Bitmask for setting attributes */;
	u32 freq /*! Calibration frequency for setting the pre-scalar */;
	u32 seed;
	u32 initial_value;
	u32 resd[8];
} crc_attr_t;

#define I_CRC_GETVERSION _IOCTL(CRC_IOC_IDENT_CHAR, I_MCU_GETVERSION)
#define I_CRC_GETINFO _IOCTLR(CRC_IOC_IDENT_CHAR, I_MCU_GETINFO, crc_info_t)
#define I_CRC_SETATTR _IOCTLW(CRC_IOC_IDENT_CHAR, I_MCU_SETATTR, crc_attr_t)
#define I_CRC_SETACTION _IOCTLW(CRC_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

/*! \brief See details below.
 * \details This request reads the value of the timer.
 * For example:
 * \code
 * crc_time_t current_time;
 * ioctl(crc_fd, I_CRC_GET, &current_time );
 * \endcode
 * \hideinitializer
 */
#define I_CRC_GET _IOCTLR(CRC_IOC_IDENT_CHAR, I_MCU_TOTAL, u32)

#define I_CRC_TOTAL 1

#ifdef __cplusplus
}
#endif

#endif /* SOS_DEV_CRC_H_ */

/*! @} */
