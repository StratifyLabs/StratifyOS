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
#include "mcu/core.h"

typedef struct {
	mcu_event_handler_t handler;
} dma_local_t;

static dma_local_t dma_local[DMA_MAX_CHANNEL] MCU_SYS_MEM;

LPC_GPDMACH_TypeDef * _mcu_dma_get_channel(uint32_t chan){
	if ( chan < 8 ){
		return (LPC_GPDMACH_TypeDef *)(LPC_GPDMACH0_BASE + chan * 0x20);
	} else {
		return NULL;
	}
}

void _mcu_dma_init(int mode){
	//Power up the DMA
	_mcu_lpc_core_enable_pwr(PCGPDMA);
	//Enable the controller
	LPC_GPDMA->Config = (1<<0);
	//Enable the interrupts
	_mcu_core_priv_enable_irq((void*)DMA_IRQn);
}

void _mcu_dma_exit(void){
	//disable the DMA
	LPC_GPDMA->Config = 0;
	//Power down the DMA
	_mcu_lpc_core_disable_pwr(PCGPDMA);
}

int _mcu_dma_halttransfer(int chan){
	LPC_GPDMACH_TypeDef * channel_regs;
	//Get the channel registers
	if ( chan < 0 ){
		return -1;
	}

	if ( chan > DMA_MAX_CHANNEL ){
		return -1;
	}

	channel_regs = _mcu_dma_get_channel(chan);
	channel_regs->CConfig = 0;
	return chan;
}


void _mcu_core_dma_isr(void){
	int i;
	//Execute the channel callbacks if they are available
	for(i=0; i < DMA_MAX_CHANNEL; i++){
		if ( LPC_GPDMA->IntTCStat & (1<<i) ){ //If there is an interrupt on the channel, check for the callback
			LPC_GPDMA->IntTCClear = (1<<i);
			_mcu_core_exec_event_handler(&(dma_local[i].handler), 0);
		}

		if ( LPC_GPDMA->IntErrStat & (1<<i) ){
			LPC_GPDMA->IntErrClr = (1<<i);
			_mcu_core_exec_event_handler(&(dma_local[i].handler), (mcu_event_t)1);
		}
	}
}

int _mcu_dma_transferlist(int operation,
		int chan,
		dma_lli_t * linked_list,
		mcu_callback_t callback,
		void * context,
		uint32_t dest_periph,
		uint32_t src_periph
		){
	LPC_GPDMACH_TypeDef * channel_regs;

	//Get the channel registers
	channel_regs = _mcu_dma_get_channel(chan);
	if (channel_regs == NULL ){
		//This is not a valid channel
		return -1;
	}

	if ( (LPC_GPDMA->EnbldChns & (1<<chan)) ){
		//Channel is busy
		return -1;
	}

	//Clear pending interrupts
	LPC_GPDMA->IntTCClear |= (1<<chan);
	LPC_GPDMA->IntErrClr |= (1<<chan);

	//Set up the transfer
	channel_regs->CSrcAddr = (uint32_t)linked_list->src;
	channel_regs->CDestAddr = (uint32_t)linked_list->dest;
	channel_regs->CControl = linked_list->ctrl;
	channel_regs->CLLI = (uint32_t)linked_list->next;

	//Set the callback value
	if( _mcu_core_priv_validate_callback(callback) < 0 ){
		return -1;
	}

	dma_local[chan].handler.callback = callback;
	dma_local[chan].handler.context = context;

	//Start the transfer
	channel_regs->CConfig = DMA_CFG_SRC_PERIPH(src_periph)|
			DMA_CFG_DEST_PERIPH(dest_periph)|
			DMA_CFG_TRANSFER_TYPE(operation)|
			DMA_CFG_ENABLE_IE|
			DMA_CFG_ENABLE_ITC|
			DMA_CFG_ENABLE_CHAN;

	return chan;
}

int _mcu_dma_transfer(int operation,
		int chan,
		void * dest,
		const void * src,
		int ctrl,
		mcu_callback_t cb,
		void * context,
		uint32_t dest_periph,
		uint32_t src_periph){
	dma_lli_t list;
	list.src = src;
	list.dest = dest;
	list.ctrl = ctrl;
	list.next = NULL;


	//If there is a callback, enable the interrupt
	if ( cb != 0 ){
		list.ctrl |= DMA_CTRL_ENABLE_TERMINAL_COUNT_INT;
	}

	return _mcu_dma_transferlist(operation, chan, &list, cb, context, dest_periph, src_periph);
}

