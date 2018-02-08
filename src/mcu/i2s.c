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

#include "variant.h"
#include "mcu/i2s.h"

MCU_DRIVER_DECLARATION(i2s, I2S_VERSION)
DEVFS_MCU_DRIVER_IOCTL_FUNCTION_TABLE(i2s, I_MCU_TOTAL + I_I2S_TOTAL, mcu_i2s_mute, mcu_i2s_unmute)
MCU_DRIVER_DECLARATION_LOCAL(i2s, I_MCU_TOTAL + I_I2S_TOTAL)


