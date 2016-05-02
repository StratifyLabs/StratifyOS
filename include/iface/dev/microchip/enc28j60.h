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

/*! \addtogroup ENC28J60 ENC28J60 SPI to Ethernet Device
 * @{
 * \ingroup MICROCHIP
 *
 * \details This is a SPI to MAC ethernet adapter.  This driver is not implemented in this version.
 *
 */

#ifndef DEV_ENC28J60_H_
#define DEV_ENC28J60_H_


#include <stdint.h>
#include "iface/device_config.h"

#include "iface/dev/eth.h"


/*! \details This defines the configuration values for
 * a SST25VF device in the device table.
 *
 *
 * \param device_name The name of the device (e.g "disk0")
 * \param port_number The SPI peripheral port number
 * \param pin_assign_value The GPIO configuration to use with the SPI bus
 * \param cs_port_value The GPIO port used for the chip select
 * \param cs_pin_value The GPIO pin used for the chip select
 * \param bitrate_value The maximum allowable bitrate for the ENC28J60
 * \param cfg_dcfg Pointer to const enc28j60_cfg_t
 * \param cfg_state Pointer to enc28j60_state_t
 * \param mode_value The access mode (usually 0666)
 * \param uid_value The User ID
 * \param gid_value The Group ID
 *
 * \hideinitializer
 *
 */
#define ENC28J60_DEVICE(device_name, port_number, pin_assign_value, cs_port_value, cs_pin_value, bitrate_value, cfg_dcfg, cfg_state, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(enc28j60), \
		.cfg.periph.port = port_number, \
		.cfg.pin_assign = pin_assign_value, \
		.cfg.pcfg.spi.mode = SPI_ATTR_MODE0, \
		.cfg.pcfg.spi.cs.port = cs_port_value, \
		.cfg.pcfg.spi.cs.pin = cs_pin_value, \
		.cfg.pcfg.spi.width = 8, \
		.cfg.pcfg.spi.format = SPI_ATTR_FORMAT_SPI, \
		.cfg.bitrate = bitrate_value, \
		.cfg.state = cfg_state, \
		.cfg.dcfg = cfg_dcfg \
}


#ifdef __cplusplus
extern "C" {
#endif

/*! \details This is the enc28j60 attributes data structure.
 *
 */
typedef struct {
	uint8_t mac_addr[8] /*! \brief the MAC address */;
	bool full_duplex /*! \brief Full duplex mode setting */;
} enc28j60_attr_t;

/*! \details This is the enc28j60 tx status attribute.
 *
 */
typedef struct MCU_PACK {
	uint16_t count;
	uint8_t status[5];
} enc28j60_txstatus_t;


#ifdef __cplusplus
}
#endif

#endif /* DEV_ENC28J60_H_ */

/*! @} */
