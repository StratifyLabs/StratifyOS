// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

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

/*! \file 
 */


#ifndef SOS_DEV_NETIF_H_
#define SOS_DEV_NETIF_H_

#include "sos/link/types.h"
#include <sdk/types.h>

#define NETIF_VERSION (0x030100)
#define NETIF_IOC_CHAR 'N'

#define NETIF_MAX_MAC_ADDRESS_SIZE 16

enum {
  NETIF_FLAG_INIT /*! When setting attributes, initializes the interface */ = (1 << 0),
  NETIF_FLAG_DEINIT /*! When setting attributes, de-initialies the interface */ =
    (1 << 1),
  NETIF_FLAG_IS_LINK_UP /*! When settings attributes, ioctl() return 1 if link is up */ =
    (1 << 2),
  NETIF_FLAG_SET_LINK_UP /*! When setting attributes, sets the link as up */ = (1 << 3),
  NETIF_FLAG_SET_LINK_DOWN /*! When setting attributes, sets the link as down */ =
    (1 << 4),
  NETIF_FLAG_SET_MAC_ADDRESS /*! Sets the MAC address populated by I_NETIF_GETINFO */ =
    (1 << 5),
  NETIF_FLAG_IS_BROADCAST = (1 << 6),
  NETIF_FLAG_IS_ETHERNET = (1 << 7),
  NETIF_FLAG_IS_ETHERNET_ARP = (1 << 8),
  NETIF_FLAG_IS_IGMP = (1 << 9),
  NETIF_FLAG_IS_MLD6 = (1 << 10),
};

typedef struct MCU_PACK {
  u32 o_flags;
  u32 o_events;
  u8 mac_address[NETIF_MAX_MAC_ADDRESS_SIZE];
  u8 mac_address_length;
  u8 resd;
  u16 mtu;
} netif_info_t;

/*! \brief Network Interface attributes
 */
typedef struct MCU_PACK {
  u32 o_flags;
  u16 mtu;
  u8 mac_address[NETIF_MAX_MAC_ADDRESS_SIZE];
} netif_attr_t;

#define I_NETIF_GETVERSION _IOCTL(NETIF_IOC_CHAR, I_MCU_GETVERSION)

/*! \brief See below for details.
 * \details This requests reads the ADC attributes.
 *
 * Example:
 * \code
 * #include <sos/dev/netif.h>
 * netif_attr_t attr;
 * int netif_fd;
 * ...
 * ioctl(netif_fd, I_NETIF_GETINFO, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_NETIF_GETINFO _IOCTLR(NETIF_IOC_CHAR, I_MCU_GETINFO, netif_info_t)

/*! \brief See below for details.
 * \details This requests writes the ETH attributes.
 *
 * Example:
 * \code
 * #include <sos/dev/netif.h>
 * netif_attr_t attr;
 * int netif_fd;
 * ...
 * ioctl(netif_fd, I_NETIF_SETATTR, &attr);
 * \endcode
 * \hideinitializer
 */
#define I_NETIF_SETATTR _IOCTLW(NETIF_IOC_CHAR, I_MCU_SETATTR, netif_attr_t)
#define I_NETIF_SETACTION _IOCTLW(NETIF_IOC_CHAR, I_MCU_SETACTION, mcu_action_t)

#define I_NETIF_TOTAL 0

#endif /* SOS_DEV_NETIF_H_ */

/*! @} */
