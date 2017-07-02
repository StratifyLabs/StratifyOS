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
#include "core_flags.h"
#include "mcu/cortexm.h"
#include "mcu/core.h"


extern void _mcu_set_sleep_mode(int * level);
#ifdef __lpc17xx__
extern LPC_GPDMACH_Type * _mcu_dma_get_channel(uint32_t chan);
#endif

int _mcu_core_sleep(core_sleep_t level){

#ifdef __lpc17xx__
	int tmp;

	LPC_GPDMACH_Type * channel_regs;

	//Wait for any DMA transactions on RAM to complete (AHB transactions are OK in idle but not deep sleep)
	for(tmp = 0; tmp < DMA_MAX_CHANNEL; tmp++){
		if ( LPC_GPDMA->EnbldChns & (1<<tmp) ){
			channel_regs = _mcu_dma_get_channel(tmp);
			if ( level >= CORE_DEEPSLEEP ){
				if ( (channel_regs->CDestAddr <= 0x10001FFF) ||
						(channel_regs->CSrcAddr <= 0x10001FFF) ){

					//wait for DMA to complete
					while(  LPC_GPDMA->EnbldChns & (1<<tmp) ){
						//this will trip the WDT if it takes too long
					}
				}
			} else {
				//wait for DMA to complete
				while(  LPC_GPDMA->EnbldChns & (1<<tmp) ){
					//this will trip the WDT if it takes too long
				}
			}
		}
	}
#endif

	mcu_core_privcall((core_privcall_t)_mcu_set_sleep_mode, &level);
	if ( level < 0 ){
		return level;
	}

	//Wait for an interrupt
	__WFI();
	return 0;
}

