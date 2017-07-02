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



#ifndef IFACE_DEV_ETH_H_
#define IFACE_DEV_ETH_H_

#include <stdint.h>
#include "ioctl.h"
#include "mcu/types.h"

#define ETH_IOC_IDENT_CHAR 'e'

enum {
	ETH_MODE_FULLDUPLEX = (1<<0),
	ETH_MODE_HALFDUPLEX = (1<<1),
};

typedef enum {
	ETH_FLAG_FULLDUPLEX = (1<<0),
	ETH_FLAG_HALFDUPLEX = (1<<1),
} eth_flags_t;

/*! \brief ETH IO Attributes
 * \details This structure defines the attributes structure
 * for configuring the ethernet port.
 */
typedef struct MCU_PACK {
	uint8_t mac_addr[8] /*! \brief the MAC address */;
	uint32_t mode /*! \brief Full duplex mode setting */;
} eth_attr_t;

typedef struct MCU_PACK {
	u32 pin_assignment;
	u32 o_flags;
	u8 mac_addr[8];
} eth_3_attr_t;

/*! \details This is the enc28j60 tx status attribute.
 *
 */
typedef struct MCU_PACK {
	uint16_t count;
	uint8_t status[5];
} eth_txstatus_t;

/*! \brief See below for details.
 * \details This requests reads the ADC attributes.
 *
 * Example:
 * \code
 * #include <dev/eth.h>
 * eth_attr_t attr;
 * int eth_fd;
 * ...
 * ioctl(eth_fd, I_ETH_GETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_ETH_ATTR _IOCTLR(ETH_IOC_IDENT_CHAR, I_GLOBAL_ATTR, eth_attr_t)
#define I_ETH_GETATTR I_ETH_ATTR

/*! \brief See below for details.
 * \details This requests writes the ETH attributes.
 *
 * Example:
 * \code
 * #include <dev/eth.h>
 * eth_attr_t attr;
 * int eth_fd;
 * ...
 * ioctl(eth_fd, I_ETH_SETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_ETH_SETATTR _IOCTLW(ETH_IOC_IDENT_CHAR, I_GLOBAL_SETATTR, eth_attr_t)
#define I_ETH_SETACTION _IOCTLW(ETH_IOC_IDENT_CHAR, I_GLOBAL_SETACTION, mcu_action_t)

/*! \brief This request starts a new packet (to be transmitted).
 * Subsequent calls to write() will store data in
 * the new packet.  The I_ETH_SENDTXPKT request will
 * send the packet.  The ctl argument is NULL.
 */
#define I_ETH_INITTXPKT _IOCTL(ETH_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 0)
#define I_ETH_INIT_TX_PKT I_ETH_INITTXPKT

/*! \brief This request transmits the current packet in the transmit buffer.
 * It returns 0 if the packet was successfully transmitted.
 */
#define I_ETH_SENDTXPKT _IOCTL(ETH_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 1)
#define I_ETH_SEND_TX_PKT I_ETH_SENDTXPKT

/*! \brief This request checks to see if there is a packet currently
 * being transmitted.  It returns true if a packet is currently being
 * transmitted and false otherwise (less than zero for an error).
 */
#define I_ETH_TXPKTBUSY _IOCTL(ETH_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 2)
#define I_ETH_TX_PKT_BUSY I_ETH_TXPKTBUSY

/*! \brief This request checks to see if a new packet is ready to
 * be read from the device.  It returns the number of bytes available
 * in the packet or less than zero for an error.  The ctl argument
 * points to a memory destination for the pointer to the next packet.
 */
#define I_ETH_RXPKTRDY _IOCTL(ETH_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 3)
#define I_ETH_RX_PKT_RDY I_ETH_RXPKTRDY

/*! \brief This request finalizes the reception of a packet.  The ctl
 * value is a pointer to the next packet address in the read buffer.  This
 * value is obtained when a packet is ready using the I_ETH_RXPKTRDY request.
 */
#define I_ETH_RXPKTCOMPLETE _IOCTL(ETH_IOC_IDENT_CHAR, I_GLOBAL_TOTAL + 4)
#define I_ETH_RX_PKT_COMPLETE I_ETH_RXPKTCOMPLETE

#define I_ETH_TOTAL (I_GLOBAL_TOTAL + 5)


#endif // IFACE_DEV_ETH_H_


/*! @} */
