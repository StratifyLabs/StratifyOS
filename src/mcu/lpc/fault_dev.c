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

#include "mcu/mcu.h"

int fault_dev_save(fault_t * fault){
	LPC_RTC->GPREG0 = fault->num;
	LPC_RTC->GPREG1 = (uint32_t)fault->pc;
	LPC_RTC->GPREG2 = (uint32_t)fault->caller;
	LPC_RTC->GPREG3 = (uint32_t)fault->handler_pc;
	LPC_RTC->GPREG4 = (uint32_t)fault->handler_caller;
	return 0;
}

int fault_dev_load(fault_t * fault){
	fault->num = LPC_RTC->GPREG0;
	fault->pc = (void*)LPC_RTC->GPREG1;
	fault->caller = (void*)LPC_RTC->GPREG2;
	fault->handler_pc = (void*)LPC_RTC->GPREG3;
	fault->handler_caller = (void*)LPC_RTC->GPREG4;
	fault->addr = (void*)0xFFFFFFFF;
	LPC_RTC->GPREG0 = 0; //clear any existing faults since it has been read
	LPC_RTC->GPREG1 = 0; //clear any existing faults since it has been read
	LPC_RTC->GPREG2 = 0; //clear any existing faults since it has been read
	LPC_RTC->GPREG3 = 0; //clear any existing faults since it has been read
	LPC_RTC->GPREG4 = 0; //clear any existing faults since it has been read
	return 0;
}

