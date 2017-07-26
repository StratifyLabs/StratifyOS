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
#ifndef SYMBOLS_H_
#define SYMBOLS_H_

#include <stdint.h>
#include "mcu/mcu.h"

extern uint32_t const symbols_table[] MCU_WEAK;

static inline uint32_t symbols_total(){
	int i;
	i = 0;
	while(symbols_table[i] != 1 ){
		i++;
	}
	return i;
}


#endif /* SYMBOLS_H_ */
