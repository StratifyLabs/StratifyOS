/* Copyright 2013; 
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

/*! \addtogroup EINT_DEV EINT
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_EMC_H_
#define _MCU_EMC_H_


#include "sos/dev/emc.h"
#include "sos/fs/devfs.h"

typedef struct MCU_PACK {
	u32 value;
} emc_event_data_t;

typedef struct MCU_PACK {
	emc_attr_t attr; //default attributes
} emc_config_t;

#ifdef __cplusplus
extern "C" {
#endif

#define MCU_EMC_IOCTL_REQUEST_DECLARATION(driver_name) \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getinfo); \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setattr); \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setaction)

#define MCU_EMC_DRIVER_DECLARATION(variant) \
	DEVFS_DRIVER_DECLARTION(variant); \
	MCU_EMC_IOCTL_REQUEST_DECLARATION(variant)

MCU_EMC_DRIVER_DECLARATION(mcu_emc_sram);
MCU_EMC_DRIVER_DECLARATION(mcu_emc_sdram);
MCU_EMC_DRIVER_DECLARATION(mcu_emc_nand);
MCU_EMC_DRIVER_DECLARATION(mcu_emc_nor);
MCU_EMC_DRIVER_DECLARATION(mcu_emc_psram);

/*#define EMC_DEFINE_ATTR(attr_flags, \
    attr_freq, \
    attr_width, \
    attr_data0_port, attr_data0_pin, \
    attr_data1_port, attr_data1_pin, \
    attr_data2_port, attr_data2_pin, \
    attr_data3_port, attr_data3_pin, \
    attr_sck_port, attr_sck_pin, \
    attr_cs_port, attr_cs_pin) \
    .o_flags = attr_flags, .freq = attr_freq, .width = attr_width, \
    .pin_assignment.data[0] = {attr_data0_port, attr_data0_pin}, \
    .pin_assignment.data[1] = {attr_data1_port, attr_data1_pin}, \
    .pin_assignment.data[2] = {attr_data2_port, attr_data2_pin}, \
    .pin_assignment.data[3] = {attr_data3_port, attr_data3_pin}, \
    .pin_assignment.sck = {attr_sck_port, attr_sck_pin}, \
    .pin_assignment.cs = {attr_cs_port, attr_cs_pin}

#define EMC_DECLARE_CONFIG(name, \
    attr_flags, \
    attr_freq, \
    attr_width, \
    attr_data0_port, attr_data0_pin, \
    attr_data1_port, attr_data1_pin, \
    attr_data2_port, attr_data2_pin, \
    attr_data3_port, attr_data3_pin, \
    attr_sck_port, attr_sck_pin, \
    attr_cs_port, attr_cs_pin) \
    qspi_config_t name##_config = { \
    .attr = { EMC_DEFINE_ATTR(attr_flags, \
    attr_freq, \
    attr_width, \
    attr_data0_port, attr_data0_pin, \
    attr_data1_port, attr_data1_pin, \
    attr_data2_port, attr_data2_pin, \
    attr_data3_port, attr_data3_pin, \
    attr_sck_port, attr_sck_pin, \
    attr_cs_port, attr_cs_pin) } \
}
*/
#ifdef __cplusplus
}
#endif

#endif /* _EMC_H_ */

/*! @} */

