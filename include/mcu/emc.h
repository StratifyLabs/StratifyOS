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
	u32 base_address;
	u32 size;
	u32 freq;
	u16 data_bus_width;
	u16 resd;
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
MCU_EMC_DRIVER_DECLARATION(mcu_emc_fmc_ahb);

#ifdef __cplusplus
}
#endif

#endif /* _EMC_H_ */

/*! @} */

