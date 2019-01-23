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

/*! \addtogroup ETH_DEV Ethernet Device Access
 * @{
 *
 * \ingroup IFACE_DEV
 *
 * \details The ethernet driver provides access to an ethernet device.
 *
 *
 * \code
 *
 * int fd;
 *
 *  //open and init ethernet driver
 *
 *  //sending packets: initialize TX, write data, send packet
 * ioctl(fd, I_ETH_INITTXPKT); //prepare transmission
 * write(fd, eth_buffer, eth_size); //write to buffers
 * ioctl(fd, I_ETH_SENDTXPKT); //send the packet
 *
 *  //receiving packets
 *  int len;
 *  uint32_t addr;
 *  len = ioctl(fd, I_ETH_RXPKTRDY, &addr);
 *  if( len > 0 ){
 *  	read(fd, eth_buffer, len);
 *  	ioctl(fd, I_ETH_RXPKTCOMPLETE, addr);
 *  }
 *
 * \endcode
 *
 */

/*! \file
 * \brief Ethernet Header File
 *
 */



#ifndef SOS_DEV_ETH_H_
#define SOS_DEV_ETH_H_

#include "mcu/types.h"

#define ETH_VERSION (0x030000)
#define ETH_IOC_IDENT_CHAR 'e'

enum {
	ETH_FLAG_SET_INTERFACE = (1<<0),
	ETH_FLAG_IS_FULLDUPLEX = (1<<1),
	ETH_FLAG_IS_HALFDUPLEX = (1<<2),
	ETH_FLAG_IS_AUTONEGOTIATION_ENABLED = (1<<3),
	ETH_FLAG_IS_SPEED_100M = (1<<4),
	ETH_FLAG_IS_SPEED_1G = (1<<5),
	ETH_FLAG_IS_MII = (1<<6),
	ETH_FLAG_IS_RMII = (1<<7),
	ETH_FLAG_GET_STATE = (1<<8),
	ETH_FLAG_SET_REGISTER = (1<<9),
	ETH_FLAG_GET_REGISTER = (1<<10),
	ETH_FLAG_STOP = (1<<11),
	ETH_FLAG_START = (1<<12)
};

typedef struct MCU_PACK {
	mcu_pin_t clk;
	mcu_pin_t txd0;
	mcu_pin_t txd1;
	mcu_pin_t tx_en;
	mcu_pin_t rxd0;
	mcu_pin_t rxd1;
	mcu_pin_t crs_dv;
	mcu_pin_t rx_er;
	mcu_pin_t unused[8]; //needs to match size of eth_mii_pin_assignment_t
} eth_rmii_pin_assignment_t;

typedef struct MCU_PACK {
	mcu_pin_t tx_clk;
	mcu_pin_t txd0;
	mcu_pin_t txd1;
	mcu_pin_t txd2;
	mcu_pin_t txd3;
	mcu_pin_t tx_en;
	mcu_pin_t tx_er;
	mcu_pin_t rx_clk;
	mcu_pin_t rxd0;
	mcu_pin_t rxd1;
	mcu_pin_t rxd2;
	mcu_pin_t rxd3;
	mcu_pin_t rx_dv;
	mcu_pin_t rx_er;
	mcu_pin_t crs;
	mcu_pin_t col;
} eth_mii_pin_assignment_t;

typedef struct MCU_PACK {
	union {
		eth_rmii_pin_assignment_t rmii;
		eth_mii_pin_assignment_t mii;
	};
	mcu_pin_t mdio;
	mcu_pin_t mdc;
} eth_pin_assignment_t;

typedef struct MCU_PACK {
	u32 o_flags;
	u32 o_events;
	u8 mac_address[10] /*! Hardware mac address */;
	u16 resd0;
	u32 resd1[8];
} eth_info_t;

/*! \brief ETH IO Attributes
 * \details This structure defines the attributes structure
 * for configuring the ethernet port.
 */
typedef struct MCU_PACK {
	u32 o_flags /*! Flag settings */;
	eth_pin_assignment_t pin_assignment /*! Pin assignement (use with ETH_FLAG_SET_INTERFACE) */;
	u8 mac_address[10] /*! the MAC address (use with ETH_FLAG_SET_INTERFACE) */;
	u16 phy_address /*! Address of PHY chip (use with ETH_FLAG_SET_INTERFACE) */;
} eth_attr_t;


#define I_ETH_GETVERSION _IOCTL(ETH_IOC_IDENT_CHAR, I_MCU_GETVERSION)

/*! \brief See below for details.
 * \details This requests reads the ADC attributes.
 *
 * Example:
 * \code
 * #include <sos/dev/eth.h>
 * eth_attr_t attr;
 * int eth_fd;
 * ...
 * ioctl(eth_fd, I_ETH_GETINFO, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_ETH_GETINFO _IOCTLR(ETH_IOC_IDENT_CHAR, I_MCU_GETINFO, eth_info_t)

/*! \brief See below for details.
 * \details This requests writes the ETH attributes.
 *
 * Example:
 * \code
 * #include <sos/dev/eth.h>
 * eth_attr_t attr;
 * int eth_fd;
 * ...
 * ioctl(eth_fd, I_ETH_SETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_ETH_SETATTR _IOCTLW(ETH_IOC_IDENT_CHAR, I_MCU_SETATTR, eth_attr_t)
#define I_ETH_SETACTION _IOCTLW(ETH_IOC_IDENT_CHAR, I_MCU_SETACTION, mcu_action_t)

#define I_ETH_SETREGISTER _IOCTLW(ETH_IOC_IDENT_CHAR, I_MCU_TOTAL, mcu_channel_t)
#define I_ETH_GETREGISTER _IOCTLRW(ETH_IOC_IDENT_CHAR, I_MCU_TOTAL+1, mcu_channel_t)

#define I_ETH_TOTAL (2)

#endif // SOS_DEV_ETH_H_


/*! @} */
