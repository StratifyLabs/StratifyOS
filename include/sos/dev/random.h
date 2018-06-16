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

/*! \addtogroup RANDOM Random
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This is the interface for accessing random number
 * generator devices.
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_RANDOM_H_
#define SOS_DEV_RANDOM_H_

#include "mcu/types.h"

#define RANDOM_VERSION (0x030000)
#define RANDOM_IOC_CHAR 'r'

enum {
    RANDOM_FLAG_SET_SEED /*! Set the seed using I_RANDOM_SETATTR */ = (1<<0),
    RANDOM_FLAG_IS_PSEUDO /*! Set in info flags is generator is pseudorandom */ = (1<<1),
    RANDOM_FLAG_IS_TRUE /*! Set in info flags is generator is a true random number generator */ = (1<<2),
    RANDOM_FLAG_ENABLE /*! Enables the random number generator */ = (1<<3),
    RANDOM_FLAG_DISABLE /*! Disables the random number generator */ = (1<<4)
};

/*! \brief Random info
 */
typedef struct MCU_PACK {
    u32 o_flags /*! Bitwise flags */;
    u32 resd[8];
} random_info_t;

/*! \brief Random attributes
 */
typedef struct MCU_PACK {
    u32 o_flags /*! Bitwise flags */;
    u32 seed /*! Seed value when setting seed */;
    u32 resd[8];
} random_attr_t;

#define I_RANDOM_GETVERSION _IOCTL(RANDOM_IOC_CHAR, I_MCU_GETVERSION)


/*! \details This request reads the random generator information.
 *
 * Example:
 * \code
 * random_info_t info;
 * ioctl(fildes, I_RANDOM_GETINFO, &info);
 * \endcode
 * \hideinitializer
 */
#define I_RANDOM_GETINFO _IOCTLR(RANDOM_IOC_CHAR, I_MCU_GETINFO, random_info_t)

/*! \details Requests the hardware to write the random generator attributes.
 *
 * Example:
 * \code
 * random_attr_t attr;
 * attr.o_flags = RANDOM_FLAG_SET_SEED;
 * attr.seed = 10;
 * ioctl(fildes, I_RANDOM_SETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_RANDOM_SETATTR _IOCTLW(RANDOM_IOC_CHAR, I_MCU_SETATTR, random_attr_t)


#endif /* SOS_DEV_RANDOM_H_ */

/*! @} */
