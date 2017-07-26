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

#include <errno.h>
#include <fcntl.h>
#include "cortexm/cortexm.h"
#include "mcu/enet.h"
#include "mcu/debug.h"
#include "mcu/core.h"

#include "enet_local.h"

#if MCU_ENET_PORTS > 0

typedef struct MCU_PACK {
	const void * buf;
	uint32_t ctrl;
} enet_descriptor_t;

typedef struct {
	mcu_event_handler_t write;
	mcu_event_handler_t read;
	enet_attr_t attr;
	uint8_t ref_count;
	enet_descriptor_t tx_desc;
	enet_descriptor_t rx_desc;
	uint32_t tx_status;
	uint32_t rx_status;
} enet_local_t;

static enet_local_t enet_local[MCU_ENET_PORTS] MCU_SYS_MEM; //this is only ever modified by priv code

#if MCU_ENET_API == 0
LPC_EMAC_Type * const enet_regs_table[MCU_ENET_PORTS] = MCU_ENET_REGS;
#else
LPC_ETHERNET_Type * const enet_regs_table[MCU_ENET_PORTS] = MCU_ENET_REGS;
#endif

u8 const enet_irqs[MCU_ENET_PORTS] = MCU_ENET_IRQS;

void mcu_enet_dev_power_on(const devfs_handle_t * handle){
	int port = handle->port;
	if ( enet_local[port].ref_count == 0 ){
		mcu_lpc_core_enable_pwr(PCENET);
		cortexm_enable_irq((void*)(u32)(enet_irqs[port]));
		enet_local[port].tx_desc.buf = NULL;
		enet_local[port].rx_desc.buf = NULL;
	}
	enet_local[port].ref_count++;


}

void mcu_enet_dev_power_off(const devfs_handle_t * handle){
	int port = handle->port;
	if ( enet_local[port].ref_count > 0 ){
		if ( enet_local[port].ref_count == 1 ){
			cortexm_disable_irq((void*)(u32)(enet_irqs[port]));
			mcu_lpc_core_disable_pwr(PCENET);
			enet_local[port].tx_desc.buf = NULL;
			enet_local[port].rx_desc.buf = NULL;
		}
		enet_local[port].ref_count--;
	}
}

int mcu_enet_dev_is_powered(const devfs_handle_t * handle){
	int port = handle->port;
	return ( enet_local[port].ref_count != 0 );
}

int mcu_enet_getinfo(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	memcpy(ctl, &(enet_local[port].attr), sizeof(enet_attr_t));
	return 0;
}

int mcu_enet_setattr(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;

#if MCU_ENET_API == 1

#else
	enet_attr_t * attr = ctl;

	LPC_EMAC_Type * regs = enet_regs_table[port];
	//set the MAC addr


	//pin select
	if( attr->pin_assign == 0 ){
		mcu_core_set_pinsel_func(1, 0, CORE_PERIPH_ENET, port);
		mcu_core_set_pinsel_func(1, 1, CORE_PERIPH_ENET, port);
		mcu_core_set_pinsel_func(1, 4, CORE_PERIPH_ENET, port);
		mcu_core_set_pinsel_func(1, 8, CORE_PERIPH_ENET, port);
		mcu_core_set_pinsel_func(1, 9, CORE_PERIPH_ENET, port);
		mcu_core_set_pinsel_func(1, 10, CORE_PERIPH_ENET, port);
		mcu_core_set_pinsel_func(1, 14, CORE_PERIPH_ENET, port);
		mcu_core_set_pinsel_func(1, 15, CORE_PERIPH_ENET, port);
		mcu_core_set_pinsel_func(1, 16, CORE_PERIPH_ENET, port);
		mcu_core_set_pinsel_func(1, 17, CORE_PERIPH_ENET, port);
	}

	regs->MAC1 = ENET_MAC1_PARF;
	regs->MAC2 = ENET_MAC2_FULLDUPLEX | ENET_MAC2_CRCEN | ENET_MAC2_PADCRCEN;

	if (attr->flags & ENET_FLAGS_USERMII) {
		regs->Command = ENET_COMMAND_FULLDUPLEX | ENET_COMMAND_PASSRUNTFRAME | ENET_COMMAND_RMII;
	} else {
#ifdef __lpc17xx
		//LPC17xx must use RMII
		errno = EINVAL;
		return -1;
#else
		regs->Command = ENET_COMMAND_FULLDUPLEX | ENET_COMMAND_PASSRUNTFRAME;
#endif
	}

	//set full or half duplex
	if( attr->flags & ENET_FLAGS_FULLDUPLEX ){
		regs->IPGT = ENET_IPGT_FULLDUPLEX;
	} else {
		regs->IPGT = ENET_IPGT_HALFDUPLEX;
	}
	regs->IPGR = ENET_IPGR_P2_DEF;
	regs->SUPP = ENET_SUPP_100Mbps_SPEED;
	regs->MAXF = ENET_ETH_MAX_FLEN;
	regs->CLRT = ENET_CLRT_DEF;

	/* Setup default filter */
	regs->Command |= ENET_COMMAND_PASSRXFILTER;


	//set MAC address
	regs->SA0 = ((uint32_t) attr->mac_addr[5] << 8) | ((uint32_t) attr->mac_addr[4]);
	regs->SA1 = ((uint32_t) attr->mac_addr[3] << 8) | ((uint32_t) attr->mac_addr[2]);
	regs->SA2 = ((uint32_t) attr->mac_addr[1] << 8) | ((uint32_t) attr->mac_addr[0]);

	regs->Command |= (ENET_COMMAND_TXENABLE | ENET_COMMAND_RXENABLE);

	/* Clear all MAC interrupts */
	regs->IntClear = 0xFFFF;
	//pENET->MODULE_CONTROL.INTCLEAR = 0xFFFF;

	//Enable the interrupt
	regs->IntEnable = 0;
#endif
	return 0;
}


int mcu_enet_setaction(const devfs_handle_t * handle, void * ctl){

	//configure an action when a packet is sent or received

	return 0;
}



int mcu_enet_dev_read(const devfs_handle_t * handle, devfs_async_t * rop){
	int port = handle->port;

	if( enet_local[port].rx_desc.buf != 0 ){
		errno = EAGAIN;
		return -1;
	}

	if( rop->nbyte == 0 ){
		return 0;
	}


	//setup the tx descriptor to transfer the packet
	enet_local[port].rx_desc.buf = rop->buf;
	enet_local[port].rx_desc.ctrl = ENET_RCTRL_SIZE(rop->nbyte) | ENET_RCTRL_INT;
	if( cortexm_validate_callback(rop->handler.callback) < 0 ){
		return -1;
	}

	enet_local[port].read.callback = rop->handler.callback;
	enet_local[port].read.context = rop->handler.context;

	//TX interrupt should be enabled

	//trigger the ethernet to send the packet
#if MCU_ENET_API == 1

#else
	LPC_EMAC_Type * regs = enet_regs_table[port];
	regs->RxDescriptor = (uint32_t)(&enet_local[port].rx_desc);
	regs->RxStatus = (uint32_t)(&enet_local[port].rx_status);
	regs->RxDescriptorNumber = 1; //one descriptor in the queue
#endif


	return 0;
}

int mcu_enet_dev_write(const devfs_handle_t * handle, devfs_async_t * wop){
	int port = handle->port;



	if( enet_local[port].tx_desc.buf != 0 ){
		errno = EAGAIN;
		return -1;
	}

	if( wop->nbyte == 0 ){
		return 0;
	}


	//setup the tx descriptor to transfer the packet
	enet_local[port].tx_desc.buf = wop->buf;
	enet_local[port].tx_desc.ctrl = ENET_TCTRL_SIZE(wop->nbyte) | ENET_TCTRL_LAST | ENET_TCTRL_INT;

	if( cortexm_validate_callback(wop->handler.callback) < 0 ){
		return -1;
	}

	enet_local[port].write.callback = wop->handler.callback;
	enet_local[port].write.context = wop->handler.context;

	//TX interrupt should be enabled

	//trigger the ethernet to send the packet
#if MCU_ENET_API == 1

#else
	LPC_EMAC_Type * regs = enet_regs_table[port];
	regs->TxDescriptor = (uint32_t)(&enet_local[port].tx_desc);
	regs->TxStatus = (uint32_t)(&enet_local[port].tx_status);
	regs->TxProduceIndex = 0;
	regs->TxDescriptorNumber = 1; //one descriptor in the queue
#endif

	return 0;
}

int mcu_enet_inittxpkt(const devfs_handle_t * handle, void * ctl){


	return 0;
}

void mcu_core_enet0_isr(int port){

	//check for a tx or an rx interrupt
	if( 1 ){ //tx interrupt
		enet_local[port].tx_desc.buf = 0;
		mcu_execute_event_handler(&(enet_local[port].write), MCU_EVENT_FLAG_WRITE_COMPLETE, 0);
	} else {
		enet_local[port].tx_desc.buf = 0;
		mcu_execute_event_handler(&(enet_local[port].read), MCU_EVENT_FLAG_DATA_READY, 0);
	}
}

#endif

