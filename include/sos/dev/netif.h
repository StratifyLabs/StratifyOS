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

/*! \addtogroup NETIF Network Interface
 * @{
 *
 *
 * \ingroup IFACE_NETIF_DEV
 *
 * \details This is the interface for network devices. These requests must be implemented
 * by any network interface (ethernet, wifi, etc) in order for LWIP to be able
 * to use the network interface.
 *
 */

/*! \file  */

#ifndef SOS_DEV_NETIF_H_
#define SOS_DEV_NETIF_H_


#include <stdint.h>

#include "ioctl.h"
#include "mcu/types.h"

#define NETIF_IOC_CHAR 'N'

/*! \brief Network Interface attributes
 */
typedef struct MCU_PACK {
	u32 o_flags;
	u16 mtu;
} netif_attr_t;


/*! \details This request initializes the network
 * interface.
 *
 * Example:
 * \code
 * ioctl(fildes, I_NETIF_INIT);
 * \endcode
 * \hideinitializer
 */
#define I_NETIF_INIT _IOCTL(NETIF_IOC_CHAR, 0)

/*! \details This request returns the number of bytes that are ready
 * to be read from the network interface.
 *
 * Example:
 * \code
 * int len;
 * len = ioctl(fildes, I_NETIF_LEN);
 * \endcode
 * \hideinitializer
 */
#define I_NETIF_LEN _IOCTL(NETIF_IOC_CHAR, 1)

#define I_NETIF_TOTAL 2

#endif /* SOS_DEV_NETIF_H_ */

/*! @} */
