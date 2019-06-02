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

/*! \addtogroup HASH Hash
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This is the interface for accesing hardware
 * hashing accelerators.
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_HASH_H_
#define SOS_DEV_HASH_H_

#include "mcu/types.h"

#define HASH_VERSION (0x030000)
#define HASH_IOC_CHAR 'h'

enum {
	HASH_FLAG_SET /*! Set the hash algorithm settings */ = (1<<0),
	HASH_FLAG_IS_MD1 /*! Sets the algorithm MD1 use with HASH_FLAG_SET */ = (1<<1),
	HASH_FLAG_IS_SHA1 /*! Sets the algorithm SHA1 use with HASH_FLAG_SET */ = (1<<2),
	HASH_FLAG_IS_MD5 /*! Sets the algorithm MD5 use with HASH_FLAG_SET */ = (1<<3),
	HASH_FLAG_IS_SHA224 /*! Sets the algorithm SHA224 use with HASH_FLAG_SET */ = (1<<4),
	HASH_FLAG_IS_SHA256 /*! Sets the algorithm SHA256 use with HASH_FLAG_SET */ = (1<<5),
	HASH_FLAG_IS_DATA_8 /*! Sets the data size to a 8-bits */ = (1<<6),
	HASH_FLAG_IS_DATA_16 /*! Sets the data size to a 16-bits */ = (1<<7),
	HASH_FLAG_IS_DATA_32 /*! Sets the data size to a 32-bits */ = (1<<8),
};

/*! \brief Hash info
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Bitwise flags */;
	u32 resd[8];
} hash_info_t;

/*! \brief Hash attributes
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Bitwise flags */;
	u32 resd[8];
} hash_attr_t;

#define I_HASH_GETVERSION _IOCTL(HASH_IOC_CHAR, I_MCU_GETVERSION)


/*! \details This request reads the random generator information.
 *
 * Example:
 * \code
 * hash_info_t info;
 * ioctl(fildes, I_HASH_GETINFO, &info);
 * \endcode
 * \hideinitializer
 */
#define I_HASH_GETINFO _IOCTLR(HASH_IOC_CHAR, I_MCU_GETINFO, hash_info_t)

/*! \details Requests the hardware to write the random generator attributes.
 *
 * Example:
 * \code
 * hash_attr_t attr;
 * attr.o_flags = HASH_FLAG_SET_SEED;
 * attr.seed = 10;
 * ioctl(fildes, I_HASH_SETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_HASH_SETATTR _IOCTLW(HASH_IOC_CHAR, I_MCU_SETATTR, hash_attr_t)


#endif /* SOS_DEV_HASH_H_ */

/*! @} */
