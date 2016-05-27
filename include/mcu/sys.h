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

/*! \addtogroup SYS_DEV SYS
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_SYS_H_
#define _MCU_SYS_H_


#include "../iface/dev/spi.h"
#include "iface/device_config.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t _mcu_sys_getsecurity() MCU_PRIV_CODE;
void _mcu_sys_setsecuritylock(bool enabled) MCU_PRIV_CODE;

#ifdef __cplusplus
}
#endif

#endif /* _MCU_SYS_H_ */
