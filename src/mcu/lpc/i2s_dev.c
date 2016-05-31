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
#include "mcu/i2s.h"
#include "mcu/debug.h"
#include "mcu/core.h"
#include "mcu/pio.h"
#include "mcu/dma.h"



#if MCU_I2S_PORTS > 0

#define WRITE_OP 0
#define READ_OP 1

//__IO uint32_t * _mcu_get_iocon_regs(int port, int pin);

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
	int * nbyte;
	int len;
	u32 * bufp;
} i2s_transfer_t;

typedef struct {
	u8 ref_count;
	i2s_transfer_t rx;
	i2s_transfer_t tx;
} i2s_local_t;

i2s_attr_t i2s_local_attr[MCU_I2S_PORTS] MCU_SYS_MEM;
i2s_local_t i2s_local[MCU_I2S_PORTS] MCU_SYS_MEM;

static void exec_callback(i2s_transfer_t * transfer, void * data);


void _mcu_i2s_dev_power_on(int port){
	if ( i2s_local[port].ref_count == 0 ){
		switch(port){
		case 0:
			_mcu_lpc_core_enable_pwr(PCI2S);
			break;
		}
		_mcu_core_priv_enable_irq((void*)(u32)(i2s_irqs[port]));
		i2s_local[port].rx.handler.callback = NULL;
		i2s_local[port].tx.handler.callback = NULL;
		i2s_local[port].rx.bufp = 0;
		i2s_local[port].tx.bufp = 0;
	}
	i2s_local[port].ref_count++;
}

void _mcu_i2s_dev_power_off(int port){
	if ( i2s_local[port].ref_count > 0 ){
		if ( i2s_local[port].ref_count == 1 ){
			_mcu_core_priv_disable_irq((void*)(u32)(i2s_irqs[port]));
			switch(port){
			case 0:
				_mcu_lpc_core_disable_pwr(PCI2S);
				break;
			}
		}
		i2s_local[port].ref_count--;
	}
}

int _mcu_i2s_dev_powered_on(int port){
	return ( i2s_local[port].ref_count != 0 );
}

int mcu_i2s_getattr(int port, void * ctl){
	memcpy(ctl, &(i2s_local_attr[port]), sizeof(i2s_attr_t));
	return 0;
}

int mcu_i2s_setattr(int port, void * ctl){
	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);
	i2s_attr_t * p = ctl;
	u32 audio_reg = 0;
	u32 bits;
	u32 bitrate;
	u32 half_period;

	bits = 8;
	if( p->mode & I2S_MODE_WORDWIDTH_16 ){
		audio_reg |= 1;
		bits = 16;
	} else if( p->mode & I2S_MODE_WORDWIDTH_32 ){
		audio_reg |= 3;
		bits = 32;
	}

	bitrate = bits * p->frequency;

	if( p->mode & I2S_MODE_MONO ){
		audio_reg |= (1<<2);
		half_period = bits/2 - 1;
	} else {
		half_period = bits - 1;
		bitrate = bitrate * 2;
	}

	audio_reg |= (half_period << 6);

	if( (p->mode & I2S_MODE_MASTER) == 0 ){
		//set slave mode
		audio_reg |= (1<<5);
	}

	//
	if( p->mode & I2S_MODE_OUTPUT ){
		i2s_regs->DAO = audio_reg;
	} else if( p->mode & I2S_MODE_INPUT ){
		i2s_regs->DAI = audio_reg;
	}

	//pin assignment
	if( p->pin_assign == 0 ){

		//4 pin mode
		i2s_regs->TXMODE = 0; //transmitter is typical with no MCLK
		i2s_regs->RXMODE = (1<<2); //share WS and bit clock with TX block

		//enable the pins
		if ( _mcu_core_set_pinsel_func(0, 6, CORE_PERIPH_I2S, port) ) return -1;
		if ( _mcu_core_set_pinsel_func(0, 7, CORE_PERIPH_I2S, port) ) return -1;
		if ( _mcu_core_set_pinsel_func(0, 8, CORE_PERIPH_I2S, port) ) return -1;
		if ( _mcu_core_set_pinsel_func(0, 9, CORE_PERIPH_I2S, port) ) return -1;

	} else if ( p->pin_assign == 1 ){

		//4 pin mode
		i2s_regs->TXMODE = (0x08); //transmitter is typical with no MCLK
		i2s_regs->RXMODE = (1<<2); //share WS and bit clock with TX block

		//enable the pins
		if ( _mcu_core_set_pinsel_func(0, 6, CORE_PERIPH_I2S, port) ) return -1;
		if ( _mcu_core_set_pinsel_func(0, 7, CORE_PERIPH_I2S, port) ) return -1;
		if ( _mcu_core_set_pinsel_func(0, 8, CORE_PERIPH_I2S, port) ) return -1;
		if ( _mcu_core_set_pinsel_func(0, 9, CORE_PERIPH_I2S, port) ) return -1;
		if ( _mcu_core_set_pinsel_func(4, 29, CORE_PERIPH_I2S, port) ) return -1;

	}


	//set the bit rate using TXRATE * 256
	u32 core_clk;
	u32 mclk;
	u8 x;
	u8 y;
	u8 min_x, min_y;
	i32 err;
	i32 tmp;

	mclk = bitrate*16;
	core_clk = _mcu_core_getclock();

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
	i2s_regs->TXBITRATE = 15;


	i2s_regs->IRQ = (5<<8)|(3<<16); //set RX and TX depth triggers


	//later add support for DMA
	//_mcu_dma_init(0);


	return 0;
}

int mcu_i2s_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;


	if( action->event & I2S_ACTION_EVENT_READ ){

		if( action->callback == 0 ){
			exec_callback(&i2s_local[port].rx, MCU_EVENT_SET_CODE(MCU_EVENT_OP_CANCELLED));
		}

		if( _mcu_core_priv_validate_callback(action->callback) < 0 ){
			return -1;
		}

		i2s_local[port].rx.handler.callback = action->callback;
		i2s_local[port].rx.handler.context = action->context;

	}

	if( action->event & I2S_ACTION_EVENT_WRITE ){

		if( action->callback == 0 ){
			exec_callback(&i2s_local[port].tx, MCU_EVENT_SET_CODE(MCU_EVENT_OP_CANCELLED));
		}

		if( _mcu_core_priv_validate_callback(action->callback) < 0 ){
			return -1;
		}

		i2s_local[port].tx.handler.callback = action->callback;
		i2s_local[port].tx.handler.context = action->context;

	}

	_mcu_core_setirqprio(i2s_irqs[port], action->prio);


	return 0;
}

int mcu_i2s_mute(int port, void * ctl){
	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);
	i2s_regs->DAO |= (1<<15);
	return 0;

}
int mcu_i2s_unmute(int port, void * ctl){
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
		i2s_regs->TXFIFO = *(i2s_local[port].tx.bufp)++ ;
		i2s_local[port].tx.len--;
	}
	return level;
}

int _mcu_i2s_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
	int port = DEVICE_GET_PORT(cfg);

	//Grab the registers
	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);

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
	if( _mcu_core_priv_validate_callback(wop->callback) < 0 ){
		return -1;
	}

	i2s_local[port].tx.handler.callback = wop->callback;
	i2s_local[port].tx.handler.context = wop->context;

	write_tx_data(port);
	i2s_regs->IRQ |= (1<<1); //enable TX interrupt
	return 0;
}


int _mcu_i2s_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	int port = DEVICE_GET_PORT(cfg);
	int nsamples;

	LPC_I2S_Type * i2s_regs = i2s_get_regs(port);

	//read from the RXFIFO

	if ( i2s_regs->IRQ & (1<<0) ){ //is receive interrupt already enabled
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
	i2s_local[port].rx.nbyte = &rop->nbyte;

	//Check the local buffer for bytes that are immediately available
	if( _mcu_core_priv_validate_callback(rop->callback) < 0 ){
		return -1;
	}

	i2s_local[port].rx.handler.callback = rop->callback;
	i2s_local[port].rx.handler.context = rop->context;

	read_rx_data(port);
	i2s_regs->IRQ |= (1<<0);  //enable the receiver interrupt
	return 0;
}


void _mcu_core_i2s0_isr(){
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

void exec_callback(i2s_transfer_t * transfer, void * data){
	transfer->bufp = 0;
	_mcu_core_exec_event_handler(&(transfer->handler), data);
}



#endif


