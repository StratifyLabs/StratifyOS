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

#ifndef MCU_CRC_H_
#define MCU_CRC_H_

#include "mcu/types.h"

u16 mcu_calc_crc16(u16 seed, const u8 * buffer, u32 nbyte);
u8 mcu_calc_crc7(u8 seed, const u8 * chr, u32 len);


#endif /* MCU_CRC_H_ */
