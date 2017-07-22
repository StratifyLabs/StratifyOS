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
 * const sst25vf_config_t sst25vf_cfg = SST25VF_DEVICE_CFG(-1, 0, -1, 0, 0, 17, 4*1024*1024); //4MB part
 *
 * const device_t devices[DEVICES_TOTAL+1] = {
 * 	...
 * 	SST25VF_DEVICE("disk0", 2, 0, 0, 16, 25000000, &sst25vf_cfg, &sst25vf_state, 0666, USER_ROOT, GROUP_ROOT),
 * 	... //other devices
 * }
 * \endcode
 */

#ifndef SOS_DEV_MICROCHIP_SST25VF_H_
#define SOS_DEV_MICROCHIP_SST25VF_H_

#include "sos/dev/drive.h"


#endif /* SOS_DEV_MICROCHIP_SST25VF_H_ */

/*! @} */
