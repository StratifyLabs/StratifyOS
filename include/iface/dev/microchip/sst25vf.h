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

/*! \addtogroup SST25VF SST25VF Serial Flash Storage
 *
 * @{
 *
 * \ingroup MICROCHIP
 *
 * \details This driver is implemented as a disk device (see \ref DISK).
 *
 *
 * Required Peripheral Hardware:
 * - 1 SPI Bus (MODE0)
 * - 1 GPIO Pin (for chip select)
 * - 2 Optional GPIO Pins for hold and write protect
 *
 * Example:
 * \code
 * #include <hwdl/sst25vf.h>
 * #include <hwpl.h>
 *
 * sst25vf_state_t sst25vf_state MCU_SYS_MEM;
 * const sst25vf_cfg_t sst25vf_cfg = SST25VF_DEVICE_CFG(-1, 0, -1, 0, 0, 17, 4*1024*1024); //4MB part
 *
 * const device_t devices[DEVICES_TOTAL+1] = {
 * 	...
 * 	SST25VF_DEVICE("disk0", 2, 0, 0, 16, 25000000, &sst25vf_cfg, &sst25vf_state, 0666, USER_ROOT, GROUP_ROOT),
 * 	... //other devices
 * }
 * \endcode
 */

#ifndef IFACE_DEV_MICROCHIP_SST25VF_H_
#define IFACE_DEV_MICROCHIP_SST25VF_H_

#include "../disk.h"


/*! \details This defines the configuration values for
 * a SST25VF device in the device table.
 *
 *
 * \param device_name The name of the device (e.g "disk0")
 * \param port_number The SPI peripheral port number
 * \param pin_assign_value The GPIO configuration to use with the SPI bus
 * \param cs_port_value The GPIO port used for the chip select
 * \param cs_pin_value The GPIO pin used for the chip select
 * \param bitrate_value The maximum allowable bitrate for the SST25VF
 * \param cfg_ptr A pointer to the configuration structure (const sst25vf_cfg_t *)
 * \param state_ptr A pointer to the state structure (sst25vf_state_t *)
 * \param mode_value The access mode (usually 0666)
 * \param uid_value The User ID
 * \param gid_value The Group ID
 *
 * \hideinitializer
 *
 */
#define SST25VF_DEVICE(device_name, port_number, pin_assign_value, cs_port_value, cs_pin_value, bitrate_value, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(sst25vf), \
		.cfg.periph.port = port_number, \
		.cfg.pin_assign = pin_assign_value, \
		.cfg.pcfg.spi.mode = SPI_ATTR_MODE0, \
		.cfg.pcfg.spi.cs.port = cs_port_value, \
		.cfg.pcfg.spi.cs.pin = cs_pin_value, \
		.cfg.pcfg.spi.width = 8, \
		.cfg.pcfg.spi.format = SPI_ATTR_FORMAT_SPI, \
		.cfg.bitrate = bitrate_value, \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}

/*! \details This defines the configuration values for the sst25vf_cfg_t
 * configuration data.
 *
 * \param hold_port The GPIO port used for the hold (-1 to ignore)
 * \param hold_pin The GPIO pin used for the hold
 * \param wp_port The GPIO port used for the write protect (-1 to ignore)
 * \param wp_pin The GPIO pin used for the write protect
 * \param miso_port The GPIO port used for the MISO
 * \param miso_pin The GPIO pin used for the MISO
 * \param device_size The number of bytes on the device
 * \hideinitializer
 */
#define SST25VF_DEVICE_CFG(hold_port, hold_pin, wp_port, wp_pin, miso_port, miso_pin, device_size) { \
		.hold.port = hold_port, \
		.hold.pin = hold_pin, \
		.wp.port = wp_port, \
		.wp.pin = wp_pin, \
		.miso.port = miso_port, \
		.miso.pin = miso_pin, \
		.size = (device_size) \
}

/*! \details This defines the device specific static configuration.
 *
 */
typedef struct {
	device_gpio_t hold /*! \brief Hold Pin */;
	device_gpio_t wp /*! \brief Write protect pin */;
	device_gpio_t miso /*! \brief The write complete pin */;
	uint32_t size /*! \brief The size of the memory on the device */;
} sst25vf_cfg_t;

/*! \details This defines the configuration values for
 * a SST25VF device in the device table.
 *
 *
 * \param device_name The name of the device (e.g "disk0")
 * \param port_number The SPI peripheral port number
 * \param pin_assign_value The GPIO configuration to use with the SPI bus
 * \param cs_port_value The GPIO port used for the chip select
 * \param cs_pin_value The GPIO pin used for the chip select
 * \param bitrate_value The maximum allowable bitrate for the SST25VF
 * \param cfg_ptr A pointer to the configuration structure (const sst25vf_cfg_t *)
 * \param state_ptr A pointer to the state structure (sst25vf_state_t *)
 * \param mode_value The access mode (usually 0666)
 * \param uid_value The User ID
 * \param gid_value The Group ID
 *
 * \hideinitializer
 *
 */
#define SST25VF_TMR_DEVICE(device_name, port_number, pin_assign_value, cs_port_value, cs_pin_value, bitrate_value, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(sst25vf_tmr), \
		.cfg.periph.port = port_number, \
		.cfg.pin_assign = pin_assign_value, \
		.cfg.pcfg.spi.mode = SPI_ATTR_MODE0, \
		.cfg.pcfg.spi.cs.port = cs_port_value, \
		.cfg.pcfg.spi.cs.pin = cs_pin_value, \
		.cfg.pcfg.spi.width = 8, \
		.cfg.pcfg.spi.format = SPI_ATTR_FORMAT_SPI, \
		.cfg.bitrate = bitrate_value, \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}


#endif /* IFACE_DEV_MICROCHIP_SST25VF_H_ */

/*! @} */
