/* Copyright 2011-2017 Tyler Gilbert;
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

#ifndef SOS_DEV_SD_H_
#define SOS_DEV_SD_H_

#include "mcu/types.h"

#define SD_VERSION (0x030000)

#define I_SD_GETVERSION _IOCTL(SD_IOC_IDENT_CHAR, I_MCU_GETVERSION)

#define I_SDSPI_STATUS _IOCTLW('S', 2, sd_spi_status_t)


#endif /* SOS_DEV_SD_H_ */
