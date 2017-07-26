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
#include <limits.h>
#include "mcu/cortexm.h"
#include "mcu/i2s.h"
#include "mcu/debug.h"
#include "mcu/core.h"
#include "mcu/pio.h"
#include "mcu/dma.h"

#if MCU_I2S_PORTS > 0

#define WRITE_OP 0
#define READ_OP 1

//__IO uint32_t * mcu_get_iocon_regs(const devfs_handle_t * handle, int pin);

static LPC_I2S_Type * const i2s_regs_table[MCU_I2S_PORTS] = MCU_I2S_REGS;
static u8 const i2s_irqs[MCU_I2S_PORTS] = MCU_I2S_IRQS;

static inline LPC_I2S_Type * i2s_get_regs(int port) MCU_ALWAYS_INLINE;
LPC_I2S_Type * i2s_get_regs(int port){
	return i2s_regs_table[port];
}

static u8 read_rx_data(int port);
static u8 write_tx_data(int port);

typedef struct {
	mcu_event_handler_t handler;
	//int * nbyte;
	int len;
	u32 * bufp;
} i2s_transfer_t;

typedef struct {
	u8 ref_count;
	u8 resd;
	i2s_transfer_t rx;
	i2s_transfer_t tx;
} i2s_local_t;

i2s_attr_t i2s_local_attr[MCU_I2S_PORTS] MCU_SYS_MEM;
i2s_local_t i2s_local[MCU_I2S_PORTS] MCU_SYS_MEM;

static void exec_callback(i2s_transfer_t * transfer, u32 o_events);


void mcu_i2s_dev_power_on(const devfs_handle_t * handle){
	int port = handle->port;
	if ( i2s_local[port].ref_count == 0 ){
		switch(port){
		case 0:
			mcu_lpc_core_enable_pwr(PCI2S);
			break;
		}
		mcu_cortexm_priv_enable_irq((void*)(u32)(i2s_irqs[port]));
		i2s_local[port].rx.handler.callback = NULL;
		i2s_local[port].tx.handler.callback = NULL;
		i2s_local[port].rx.bufp = 0;
		i2s_local[port].tx.bufp = 0;
	}
	i2s_local[port].ref_count++;
}

void mcu_i2s_dev_power_off(const devfs_handle_t * handle){
	int port = handle->port;
	if ( i2s_local[port].ref_count > 0 ){
		if ( i2s_local[port].ref_count == 1 ){
			mcu_cortexm_priv_disable_irq((void*)(u32)(i2s_irqs[port]));
			switch(port){
			case 0:
				mcu_lpc_core_disable_pwr(PCI2S);
				break;
			}
		}
		i2s_local[port].ref_count--;
	}
}

int mcu_i2s_dev_is_powered(const devfs_handle_t * handle){
	int port = handle->port;
	return ( i2s_local[port].ref_count != 0 );
}

int mcu_i2s_getinfo(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	memcpy(ctl, &(i2s_local_attr[port]), sizeof(i2s_attr_t));
	return 0;
}

int mcu_i2s_setattr(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);
	i2s_attr_t * attr = ctl;
	u32 audio_reg = (1<<4); //start by holding the i2s in reset
	u32 bits;
	u32 bitrate;
	u32 half_period;
	u32 o_flags = attr->o_flags;

	bits = 8;
	if( o_flags & I2S_FLAG_IS_WIDTH_16 ){
		audio_reg |= 1;
		bits = 16;
	} else if( o_flags & I2S_FLAG_IS_WIDTH_32 ){
		audio_reg |= 3;
		bits = 32;
	}

	bitrate = bits * attr->freq;

	if( o_flags & I2S_FLAG_IS_MONO ){
		audio_reg |= (1<<2);
		half_period = bits/2 - 1;
	} else {
		half_period = bits - 1;
		bitrate = bitrate * 2;
	}

	audio_reg |= (half_period << 6);

	if( (o_flags & I2S_FLAG_SET_MASTER) == 0 ){
		//set slave mode because I2S_MODE_MASTER is zero
		audio_reg |= (1<<5);
	}

	//enable the transmitter
	if( o_flags & I2S_FLAG_IS_TRANSMITTER ){
		i2s_regs->DAO = audio_reg;
	} else {
		i2s_regs->DAO = (1<<3);
	}

	//enable the receiver
	if( o_flags & I2S_FLAG_IS_RECEIVER ){
		i2s_regs->DAI = audio_reg;
	} else {
		i2s_regs->DAI = (1<<3);
	}

	i2s_regs->TXMODE = 0; //transmitter is typical with no MCLK
	i2s_regs->RXMODE = (1<<2); //share WS and bit clock with TX block

	if( mcu_core_set_pin_assignment(&(attr->pin_assignment),
			MCU_PIN_ASSIGNMENT_COUNT(i2s_pin_assignment_t),
			CORE_PERIPH_I2S,
			port) < 0 ){
		return -1;
	}

	if( o_flags & I2S_FLAG_IS_MCLK_ENABLED ){
		i2s_regs->TXMODE |= (1<<3);
	}


	// \todo Add pin assign 1 that uses RX WS and RX CLK -- RX WS can be 0.5 or 0.24


	//set the bit rate using TXRATE * 256
	u32 core_clk;
	u32 mclk;
	u8 x;
	u8 y;
	u8 min_x, min_y;
	s32 err;
	s32 tmp;

	mclk = bitrate*attr->mck_mult;
	core_clk = mcu_board_config.core_periph_freq;

	min_x = 1;
	min_y = 1;
	err = INT_MAX;
	for(x = 1; x < 255; x++){
		for(y=x; y < 255; y++){
			tmp = core_clk - (mclk * 2 / x * y);
			if( tmp < 0 ){
				tmp *= -1;
			}
			if( tmp < err ){
				err = tmp;
				min_x = x;
				min_y = y;
			}
		}
	}

	//This is the MCLK rate
	i2s_regs->TXRATE = min_y | (min_x<<8);

	//now set bit clock which is TXRATE / (TX_BITRATE+1) up to 64
	i2s_regs->TXBITRATE = attr->mck_mult-1;


	i2s_regs->IRQ = (4<<8)|(4<<16); //set RX and TX depth triggers


	i2s_regs->DAO &= ~(1<<4);
	i2s_regs->DAI &= ~(1<<4);


	//later add support for DMA
	//mcu_dma_init(0);


	return 0;
}

int mcu_i2s_setaction(const devfs_handle_t * handle, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	int port = handle->port;


	if( action->o_events & MCU_EVENT_FLAG_DATA_READY ){

		if( action->handler.callback == 0 ){
			exec_callback(&i2s_local[port].rx, MCU_EVENT_FLAG_CANCELED);
		}

		if( mcu_cortexm_priv_validate_callback(action->handler.callback) < 0 ){
			return -1;
		}

		i2s_local[port].rx.handler.callback = action->handler.callback;
		i2s_local[port].rx.handler.context = action->handler.context;

	}

	if( action->o_events & MCU_EVENT_FLAG_WRITE_COMPLETE ){

		if( action->handler.callback == 0 ){
			exec_callback(&i2s_local[port].tx, MCU_EVENT_FLAG_CANCELED);
		}

		if( mcu_cortexm_priv_validate_callback(action->handler.callback) < 0 ){
			return -1;
		}

		i2s_local[port].tx.handler.callback = action->handler.callback;
		i2s_local[port].tx.handler.context = action->handler.context;

	}

	mcu_cortexm_set_irq_prio(i2s_irqs[port], action->prio);


	return 0;
}

int mcu_i2s_mute(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);
	i2s_regs->DAO |= (1<<15);
	return 0;

}

int mcu_i2s_unmute(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);
	i2s_regs->DAO &= ~(1<<15);
	return 0;
}

u8 read_rx_data(int port){
	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);
	u8 level = ((i2s_regs->STATE >> 8)  & 0x0F);
	u8 i;
	if( level > i2s_local[port].rx.len ){
		level = i2s_local[port].rx.len;
	}
	for(i=0; i < level; i++){
		*(i2s_local[port].rx.bufp)++ = i2s_regs->RXFIFO;
		i2s_local[port].rx.len--;
	}
	return level;
}

u8 write_tx_data(int port){
	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);
	u8 level = 8 - ((i2s_regs->STATE >> 16)  & 0x0F);
	u8 i;
	if( level > i2s_local[port].tx.len ){
		level = i2s_local[port].tx.len;
	}

	for(i=0; i < level; i++){
		i2s_regs->TXFIFO = *(i2s_local[port].tx.bufp);

		i2s_local[port].tx.bufp++;
		i2s_local[port].tx.len--;
	}

	return level;
}

int mcu_i2s_dev_write(const devfs_handle_t * cfg, devfs_async_t * wop){
	int port = DEVFS_GET_PORT(cfg);

	//Grab the registers
	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);

	if( i2s_regs->DAO & (1<<3) ){
		//output is not enabled
		errno = EIO;
		return -1;
	}

	if ( i2s_regs->IRQ & (1<<1) ){ //is tx interrupt already enabled
		errno = EAGAIN;
		return -1;
	}

	if ( wop->nbyte == 0 ){
		return 0;
	}

	//Initialize variables
	i2s_local[port].tx.bufp = wop->buf;
	i2s_local[port].tx.len = wop->nbyte/4;

	//Check the local buffer for bytes that are immediately available
	if( mcu_cortexm_priv_validate_callback(wop->handler.callback) < 0 ){
		return -1;
	}

	i2s_local[port].tx.handler.callback = wop->handler.callback;
	i2s_local[port].tx.handler.context = wop->handler.context;

	write_tx_data(port);
	i2s_regs->IRQ |= (1<<1); //enable TX interrupt -- interrupt fires when FIFO is ready for more data
	return 0;
}


int mcu_i2s_dev_read(const devfs_handle_t * cfg, devfs_async_t * rop){
	int port = DEVFS_GET_PORT(cfg);
	int nsamples;
	int len;

	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);

	//read from the RXFIFO
	if( i2s_regs->DAI & (1<<3) ){
		//input is not enabled
		errno = EIO;
		return -1;
	}

	if ( i2s_regs->IRQ & (1<<0) ){ //is receive interrupt already enabled - another context is using the IRQ
		errno = EAGAIN;
		return -1;
	}

	if ( rop->nbyte == 0 ){
		return 0;
	}

	//initialize the transfer
	nsamples = rop->nbyte/4;
	i2s_local[port].rx.bufp = rop->buf;
	i2s_local[port].rx.len = nsamples;

	//Check the local buffer for bytes that are immediately available
	read_rx_data(port);
	len = nsamples - i2s_local[port].rx.len;

	//for non-blocking operations, return number of bytes read or -1 if nothing is available
	if( rop->flags & O_NONBLOCK ){

		if( len == 0 ){
			//if no bytes were read, return a try again error
			i2s_local[port].rx.handler.callback = NULL;
			i2s_local[port].rx.bufp = NULL;
			rop->nbyte = 0; //no samples were read
			errno = EAGAIN;
			len = -1;
		} else {
			len = len*4; //the number of bytes is the samples * 4
		}

	} else if( len != nsamples ){
		//for blocking operations wait until the entire buffer is read then call the callback
		if( mcu_cortexm_priv_validate_callback(rop->handler.callback) < 0 ){
			return -1;
		}

		i2s_local[port].rx.handler.callback = rop->handler.callback;
		i2s_local[port].rx.handler.context = rop->handler.context;

		i2s_regs->IRQ |= (1<<0);  //enable the receiver interrupt
		len = 0;
	}

	return len;
}


void mcu_core_i2s0_isr(){
	int port = 0;
	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);
	if( i2s_local[port].rx.bufp ){
		read_rx_data(port);
		if( i2s_local[port].rx.len == 0 ){
			i2s_regs->IRQ &= ~(1<<0); //disable RX interrupt
			exec_callback(&(i2s_local[port].rx), 0);
		}
	}

	if( i2s_local[port].tx.bufp ){
		write_tx_data(port);
		if( i2s_local[port].tx.len == 0 ){
			i2s_regs->IRQ &= ~(1<<1); //disable TX interrupt
			exec_callback(&(i2s_local[port].tx), 0);
		}
	}
}

void exec_callback(i2s_transfer_t * transfer, u32 o_events){
	transfer->bufp = 0;
	mcu_execute_event_handler(&(transfer->handler), o_events, 0);
}



#endif


