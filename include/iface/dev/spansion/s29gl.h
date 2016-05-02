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

/*! \addtogroup S29GL S29GL NOR Flash Chip
 *
 * @{
 *
 * \ingroup SPANSION
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
 * #include <hwdl/s29gl.h>
 * #include <hwpl.h>
 *
 * s29gl_state_t s29gl_state MCU_SYS_MEM;
 * const s29gl_cfg_t s29gl_cfg = S29GL_DEVICE_CFG(-1, 0, -1, 0, 0, 17, 4*1024*1024); //4MB part
 *
 * const device_t devices[DEVICES_TOTAL+1] = {
 * 	...
 * 	S29GL_DEVICE("disk0", 2, 0, 0, 16, 25000000, &s29gl_cfg, &s29gl_state, 0666, USER_ROOT, GROUP_ROOT),
 * 	... //other devices
 * }
 * \endcode
 */

#ifndef DEV_S29GL_H_
#define DEV_S29GL_H_

#include "../disk.h"


/*! \details This defines the configuration values for
 * a S29GL device in the device table.
 *
 *
 * \param device_name The name of the device (e.g "disk0")
 * \param port_number The SPI peripheral port number
 * \param pin_assign_value The GPIO configuration to use with the SPI bus
 * \param cs_port_value The GPIO port used for the chip select
 * \param cs_pin_value The GPIO pin used for the chip select
 * \param bitrate_value The maximum allowable bitrate for the S29GL
 * \param cfg_ptr A pointer to the configuration structure (const s29gl_cfg_t *)
 * \param state_ptr A pointer to the state structure (s29gl_state_t *)
 * \param mode_value The access mode (usually 0666)
 * \param uid_value The User ID
 * \param gid_value The Group ID
 *
 * \hideinitializer
 *
 */
#define S29GL_DEVICE(device_name, port_number, bitrate_value, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(s29gl), \
		.cfg.periph.port = port_number, \
		.cfg.pin_assign = 0, \
		.cfg.bitrate = bitrate_value, \
		.cfg.state = state_ptr, \
		.cfg.dcfg = cfg_ptr \
}

/*! \details This defines the configuration values for the s29gl_cfg_t
 * configuration data.
 *
 * \hideinitializer
 */
#define S29GL_DEVICE_CFG(address_mask, device_size, output_compare) { \
		.addr_mask = address_mask, \
		.oc = output_compare, \
		.size = (device_size) \
}

/*! \details This defines the device specific static configuration.
 *
 */
typedef struct {
	uint32_t addr_mask /*! \brief Address mask for accessing device */;
	uint32_t size /*! \brief The size of the memory on the device */;
	int8_t oc /*! Output compare channel used for timing writes */;
} s29gl_cfg_t;

/*! \details This defines the configuration values for
 * a S29GL device in the device table.
 *
 *
 * \param device_name The name of the device (e.g "disk0")
 * \param port_number The SPI peripheral port number
 * \param pin_assign_value The GPIO configuration to use with the SPI bus
 * \param cs_port_value The GPIO port used for the chip select
 * \param cs_pin_value The GPIO pin used for the chip select
 * \param bitrate_value The maximum allowable bitrate for the S29GL
 * \param cfg_ptr A pointer to the configuration structure (const s29gl_cfg_t *)
 * \param state_ptr A pointer to the state structure (s29gl_state_t *)
 * \param mode_value The access mode (usually 0666)
 * \param uid_value The User ID
 * \param gid_value The Group ID
 *
 * \hideinitializer
 *
 */
#define S29GL_TMR_DEVICE(device_name, port_number, pin_assign_value, cs_port_value, cs_pin_value, bitrate_value, cfg_ptr, state_ptr, mode_value, uid_value, gid_value) { \
		.name = device_name, \
		DEVICE_MODE(mode_value, uid_value, gid_value, S_IFBLK), \
		DEVICE_DRIVER(s29gl_tmr), \
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


#endif /* DEV_S29GL_H_ */

/*! @} */
