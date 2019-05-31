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

/*! \addtogroup CRYPT Cryptography
 * @{
 *
 *
 * \ingroup IFACE_DEV
 *
 * \details This is the interface for accessing cryptographic hardware.
 *
 *
 */

/*! \file  */

#ifndef SOS_DEV_CRYPT_H_
#define SOS_DEV_CRYPT_H_

#include "mcu/types.h"

#define CRYPT_VERSION (0x030000)
#define CRYPT_IOC_CHAR '?'

enum {
	CRYPT_FLAG_SET_CIPHER /*! Sets the current cipher attributes */ = (1<<0),
	CRYPT_FLAG_IS_AES_ECB /*! Sets the cipher to AES ECB */ = (1<<1),
	CRYPT_FLAG_IS_AES_CBC /*! Sets the cipher to AES CBC */ = (1<<2),
	CRYPT_FLAG_IS_AES_CTR /*! Sets the cipher to AES CTR */ = (1<<3),
	CRYPT_FLAG_IS_AES_GCM /*! Sets the cipher to AES GCM */ = (1<<4),
	CRYPT_FLAG_IS_AES_CCM /*! Sets the cipher to AES GCM */ = (1<<5),
	CRYPT_FLAG_IS_AES_128 /*! Sets the cipher to a 128-bit key */ = (1<<7),
	CRYPT_FLAG_IS_AES_192 /*! Sets the cipher to a 128-bit key */ = (1<<8),
	CRYPT_FLAG_IS_AES_256 /*! Sets the cipher to a 128-bit key */ = (1<<9),
	CRYPT_FLAG_IS_DATA_1 /*! Sets the data size to a 1-bit string */ = (1<<10),
	CRYPT_FLAG_IS_DATA_8 /*! Sets the data size to a 8-bits */ = (1<<11),
	CRYPT_FLAG_IS_DATA_16 /*! Sets the data size to a 16-bits */ = (1<<12),
	CRYPT_FLAG_IS_DATA_32 /*! Sets the data size to a 32-bits */ = (1<<13),
	CRYPT_FLAG_SET_MODE /*! Sets the cipher to encryption mode for use with CRYPT_FLAG_IS_ENCRYPT or CRYPT_FLAG_IS_DECRYPT */ = (1<<14),
	CRYPT_FLAG_IS_ENCRYPT /*! Sets the cipher to encryption mode for use with CRYPT_FLAG_SET_MODE */ = (1<<15),
	CRYPT_FLAG_IS_DECRYPT /*! Sets the cipher to decryption mode for use with CRYPT_FLAG_SET_MODE */ = (1<<16),
};

/*! \brief Random info
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Bitwise flags */;
	u8 max_header_size;
	u32 resd[8];
} crypt_info_t;

/*! \brief Random attributes
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Bitwise flags */;
	u8 key[32] /*! encryption key up to 256 bits */;
	u8 iv[32] /*! initialization vector up to 256 bits */;
	u8 iv_size;
	u8 header_size;
	u32 resd[8];
} crypt_attr_t;

#define I_CRYPT_GETVERSION _IOCTL(CRYPT_IOC_CHAR, I_MCU_GETVERSION)


/*! \details This request reads the random generator information.
 *
 * Example:
 * \code
 * crypt_info_t info;
 * ioctl(fildes, I_CRYPT_GETINFO, &info);
 * \endcode
 * \hideinitializer
 */
#define I_CRYPT_GETINFO _IOCTLR(CRYPT_IOC_CHAR, I_MCU_GETINFO, crypt_info_t)

/*! \details Requests the hardware to write the random generator attributes.
 *
 * Example:
 * \code
 * crypt_attr_t attr;
 * attr.o_flags = CRYPT_FLAG_SET_SEED;
 * attr.seed = 10;
 * ioctl(fildes, I_CRYPT_SETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_CRYPT_SETATTR _IOCTLW(CRYPT_IOC_CHAR, I_MCU_SETATTR, crypt_attr_t)


#endif /* SOS_DEV_CRYPT_H_ */

/*! @} */
