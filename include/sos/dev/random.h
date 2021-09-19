// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

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

/*! \file 
 */


#ifndef SOS_DEV_RANDOM_H_
#define SOS_DEV_RANDOM_H_

#include <sdk/types.h>

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
