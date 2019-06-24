/* Copyright 2011 MCU_ROOT_CODE; 
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

/*! \addtogroup QSPI_DEV QSPI
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_QSPI_H_
#define _MCU_QSPI_H_

#include "sos/dev/qspi.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
	u32 value;
} qspi_event_data_t;

typedef struct MCU_PACK {
	qspi_attr_t attr; //default attributes
} qspi_config_t;


#define MCU_QSPI_IOCTL_REQUEST_DECLARATION(driver_name) \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getinfo); \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setattr); \
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setaction);\
	DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, execcommand)

#define MCU_QSPI_DRIVER_DECLARATION(variant) \
	DEVFS_DRIVER_DECLARTION(variant); \
	MCU_QSPI_IOCTL_REQUEST_DECLARATION(variant)

MCU_QSPI_DRIVER_DECLARATION(mcu_qspi);
MCU_QSPI_DRIVER_DECLARATION(mcu_qspi_dma);



#ifdef __cplusplus
}
#endif


#endif /* _MCU_QSPI_H_ */

/*! @} */
