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

#include "device/sys.h"


extern const uint32_t * mcu_core_vector_table[];

static bool security_lock MCU_SYS_MEM;
static bool security_lock = true;

uint32_t mcu_sys_getsecurity(){
	if( security_lock == true ){
		return (u32)mcu_core_vector_table[9];
	}
	return 0xFFFFFFFF; //all allowed
}

void mcu_sys_setsecuritylock(bool enabled){
	security_lock = enabled;
}
