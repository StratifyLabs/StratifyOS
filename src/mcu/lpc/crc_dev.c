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


#include "mcu/types.h"
#include "mcu/crc.h"
#include "mcu/arch.h"

u16 mcu_calc_crc16(u16 seed, const u8 * buffer, u32 nbyte){
#if defined LPC_CRC
	size_t i;
	LPC_CRC->MODE = 0x00; //CRC-CCITT
	LPC_CRC->SEED = seed;
	for(i=0; i < nbyte; i++){
		LPC_CRC->WR_DATA_BYTE.DATA = buffer[i];
	}
	return LPC_CRC->SUM;
#else
	return 0;
#endif
}

u8 mcu_calc_crc7(u8 seed, const u8 * chr, u32 len){
	int i,a;
	unsigned char crc,data;
	crc=seed;
	for(a=0; a<len; a++){
		data=chr[a];
		for (i=0;i<8;i++) {
			crc <<= 1;
			if ((data & 0x80)^(crc & 0x80)){
				crc ^=0x09;
			}
			data <<= 1;
		}
	}
	crc=(crc<<1)|1;
	return(crc);
}
