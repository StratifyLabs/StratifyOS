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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */


#include <errno.h>
#include <fcntl.h>
#include "mcu/cortexm.h"
#include "mcu/spi.h"
#include "mcu/pio.h"
#include "ssp_flags.h"
#include "ssp_flags.h"
#include "mcu/core.h"
#include "mcu/debug.h"

#if MCU_SSP_PORTS > 0

typedef struct {
	char * volatile rx_buf;
	char * volatile tx_buf;
	volatile int size;
	void * duplex_mem;
	int ret;
	uint8_t pin_assign;
	uint8_t width;
	uint8_t ref_count;
	mcu_event_handler_t handler;
} ssp_local_t;

static ssp_local_t ssp_local[MCU_SSP_PORTS] MCU_SYS_MEM;

static int ssp_port_transfer(const devfs_handle_t * handle, int is_read, devfs_async_t * dop);
static void ssp_fill_tx_fifo(int port, LPC_SSP_Type * regs);
static void ssp_empty_rx_fifo(int port, LPC_SSP_Type * regs);
static int byte_swap(int port, int byte);

static LPC_SSP_Type * const ssp_regs_table[MCU_SSP_PORTS] = MCU_SSP_REGS;
static u8 const ssp_irqs[MCU_SSP_PORTS] = MCU_SSP_IRQS;

#ifdef LPCXX7X_8X
static void enable_pin(int pio_port, int pio_pin) MCU_PRIV_CODE;
void enable_pin(int pio_port, int pio_pin){
	pio_attr_t pattr;
	pattr.o_pinmask = (1<<pio_pin);
	pattr.o_flags = PIO_FLAG_SET_OUTPUT;
	mcu_pio_setattr(pio_port, &pattr);
}
#endif

void mcu_ssp_dev_power_on(const devfs_handle_t * handle){
	int port = handle->port;
	if ( ssp_local[port].ref_count == 0 ){

		mcu_cortexm_priv_enable_irq((void*)(u32)(ssp_irqs[port]));

		switch(port){
		case 0:
			mcu_lpc_core_enable_pwr(PCSSP0);
			break;

		case 1:
			mcu_lpc_core_enable_pwr(PCSSP1);
			break;

#if MCU_SSP_PORTS > 2
		case 2:
			mcu_lpc_core_enable_pwr(PCSSP2);
			break;
#endif
		}
		ssp_local[port].duplex_mem = NULL;
		ssp_local[port].handler.callback = NULL;
	}
	ssp_local[port].ref_count++;

}

void mcu_ssp_dev_power_off(const devfs_handle_t * handle){
	int port = handle->port;
	if ( ssp_local[port].ref_count > 0 ){
		if ( ssp_local[port].ref_count == 1 ){

			mcu_cortexm_priv_disable_irq((void*)(u32)(ssp_irqs[port]));

			switch(port){
			case 0:
#if defined __lpc13uxx || __lpc13xx
				LPC_SYSCON->SSP0CLKDIV = 0;
				LPC_SYSCON->PRESETCTRL &= ~(1<<0);
#endif
				mcu_lpc_core_disable_pwr(PCSSP0);
				break;
			case 1:
#if defined __lpc13uxx || __lpc13xx
				LPC_SYSCON->SSP0CLKDIV = 0;
				LPC_SYSCON->PRESETCTRL &= ~(1<<2);
#endif
				mcu_lpc_core_disable_pwr(PCSSP1);
				break;
#ifdef LPCXX7X_8X
			case 2:
				mcu_lpc_core_disable_pwr(PCSSP2);
				break;
#endif
			}
		}
		ssp_local[port].ref_count--;
	}
}

int mcu_ssp_dev_is_powered(const devfs_handle_t * handle){
	int port = handle->port;
	return ( ssp_local[port].ref_count != 0 );
}


int mcu_ssp_getinfo(const devfs_handle_t * handle, void * ctl){
	spi_info_t * info = ctl;

	//set flags
	info->o_flags = 0;


	return 0;
}

int mcu_ssp_setattr(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	LPC_SSP_Type * regs;
	uint32_t cr0, cr1, cpsr;
	uint32_t tmp;
	spi_attr_t * attr = ctl;
	u32 o_flags = attr->o_flags;
	u32 mode;
	int i;

	regs = ssp_regs_table[port];

	if( o_flags & SPI_FLAG_SET_MASTER ){

		if( attr->freq == 0 ){
			errno = EINVAL;
			return -1 - offsetof(spi_attr_t, freq);
		}

		if ( (attr->width < 4) && (attr->width > 16) ){
			errno = EINVAL;
			return -1 - offsetof(spi_attr_t, width);
		}

		mode = 0;
		if( o_flags & SPI_FLAG_MODE1 ){
			mode = 1;
		} else if( o_flags & SPI_FLAG_MODE2 ){
			mode = 2;
		} else if( o_flags & SPI_FLAG_MODE3 ){
			mode = 3;
		}

		cr0 = 0;
		cr1 = (1<<1); //set the enable

		tmp = mcu_board_config.core_periph_freq / attr->freq;
		tmp = ( tmp > 255 ) ? 254 : tmp;
		tmp = ( tmp < 2 ) ? 2 : tmp;
		if ( tmp & 0x01 ){
			tmp++; //round the divisor up so that actual is less than the target
		}
		cpsr = tmp;

		if ( mode & 0x01 ){
			cr0 |= (1<<7);
		}
		if ( mode & 0x02 ){
			cr0 |= (1<<6);
		}

		cr0 |= ( attr->width - 1);

		//default mode is SPI
		if ( o_flags & SPI_FLAG_FORMAT_TI ){
			cr0 |= (1<<4);
		} else if ( o_flags & SPI_FLAG_FORMAT_MICROWIRE ){
			cr0 |= (1<<5);
		}

		for(i=0; i < MCU_PIN_ASSIGNMENT_COUNT(spi_pin_assignment_t); i++){
			const mcu_pin_t * pin = mcu_pin_at(&(attr->pin_assignment), i);
			if( mcu_is_port_valid(pin->port) ){
#ifdef LPCXX7X_8X
				if( pin->port == 0 ){
					if( (pin->pin == 7) ||
							(pin->pin == 8) ||
							(pin->pin == 9) ){
						enable_pin(pin->port,pin->pin);
					}
				}
#endif
				if ( mcu_core_set_pinsel_func(pin->port, pin->pin, CORE_PERIPH_SSP, port) ){
					return -1;  //faile to set pin function
				}
			}
		}

		regs->CR0 = cr0;
		regs->CR1 = cr1;
		regs->CPSR = cpsr;
		regs->IMSC = 0;

	}

	return 0;
}

int mcu_ssp_swap(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	return byte_swap(port, (int)ctl);
}

int mcu_ssp_setduplex(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	ssp_local[port].duplex_mem = (void * volatile)ctl;
	return 0;
}

static void exec_callback(int port, LPC_SSP_Type * regs, u32 o_events){
	regs->IMSC &= ~(SSPIMSC_RXIM|SSPIMSC_RTIM); //Kill the interrupts
	mcu_execute_event_handler(&(ssp_local[port].handler), o_events, 0);
}


int mcu_ssp_setaction(const devfs_handle_t * handle, void * ctl){
	int port = handle->port;
	mcu_action_t * action = (mcu_action_t*)ctl;
	LPC_SSP_Type * regs;
	regs = ssp_regs_table[port];

	if( action->handler.callback == 0 ){
		if ( regs->IMSC & (SSPIMSC_RXIM|SSPIMSC_RTIM) ){
			exec_callback(port, regs, MCU_EVENT_FLAG_CANCELED);
		}
		ssp_local[port].handler.callback = 0;
		return 0;
	}

	if( mcu_cortexm_priv_validate_callback(action->handler.callback) < 0 ){
		return -1;
	}

	ssp_local[port].handler.callback = action->handler.callback;
	ssp_local[port].handler.context = action->handler.context;

	mcu_cortexm_set_irq_prio(ssp_irqs[port], action->prio);


	return 0;
}

int byte_swap(int port, int byte){
	LPC_SSP_Type * regs;
	regs = ssp_regs_table[port];

	//make sure the RX fifo is empty
	while( regs->SR & SSPSR_RNE ){
		byte = regs->DR;
	}

	regs->DR = byte;

	while ( (regs->SR & SSPSR_BSY)  || !(regs->SR & SSPSR_RNE) ){
		;
	}

	byte = regs->DR; //read the byte to empty the RX FIFO

	return byte;

}

int mcu_ssp_dev_write(const devfs_handle_t * handle, devfs_async_t * wop){
	return ssp_port_transfer(handle, 0, wop);
}

int mcu_ssp_dev_read(const devfs_handle_t * handle, devfs_async_t * rop){
	return ssp_port_transfer(handle, 1, rop);
}

void ssp_fill_tx_fifo(int port, LPC_SSP_Type * regs){
	int size = 0;
	u8 data;
	while ( (regs->SR & SSPSR_TNF) && ssp_local[port].size && (size < 4) ){
		if ( ssp_local[port].tx_buf != NULL ){
			data = *ssp_local[port].tx_buf++;
		} else {
			//fill with dummy data
			data = 0xFF;
		}

		regs->DR = data;
		ssp_local[port].size--;
		size++; //only send 4 bytes at a time so that the RX can keep up
	}
}

void ssp_empty_rx_fifo(int port, LPC_SSP_Type * regs){
	u8 data;
	while ( regs->SR & SSPSR_RNE ){
		data = regs->DR;
		if ( ssp_local[port].rx_buf != NULL ){
			*ssp_local[port].rx_buf++ = data; //save the dat
		}
	}
}

void mcu_core_ssp_isr(int port){
	LPC_SSP_Type * regs;
	regs = ssp_regs_table[port];
	regs->ICR |= (1<<SSPICR_RTIC);
	ssp_empty_rx_fifo(port, regs);
	ssp_fill_tx_fifo(port, regs);
	if ( !(regs->SR & (SSPSR_RNE)) && !(regs->SR & SSPSR_BSY) && (ssp_local[port].size == 0) ){ //empty receive fifo and not busy transmitting
		exec_callback(port, regs, 0);
	}
}

void mcu_core_ssp0_isr(){
	mcu_core_ssp_isr(0);
}

void mcu_core_ssp1_isr(){
	mcu_core_ssp_isr(1);
}

void mcu_core_ssp2_isr(){
	mcu_core_ssp_isr(2);
}

int ssp_port_transfer(const devfs_handle_t * handle, int is_read, devfs_async_t * dop){
	int port = handle->port;
	int size;
	LPC_SSP_Type * regs;
	size = dop->nbyte;


	regs = ssp_regs_table[port];

	//Check to see if SSP port is busy
	if ( regs->IMSC & (SSPIMSC_RXIM|SSPIMSC_RTIM) ){
		errno = EAGAIN;
		return -1;
	}

	if ( size == 0 ){
		return 0;
	}

	if ( is_read ){
		ssp_local[port].rx_buf = dop->buf;
		ssp_local[port].tx_buf = ssp_local[port].duplex_mem;
	} else {
		ssp_local[port].tx_buf = dop->buf;
		ssp_local[port].rx_buf = ssp_local[port].duplex_mem;
	}
	ssp_local[port].size = size;

	if( mcu_cortexm_priv_validate_callback(dop->handler.callback) < 0 ){
		return -1;
	}

	//empty RX fifo
	u8 byte;
	while( regs->SR & SSPSR_RNE ){
		byte = regs->DR;
	}

	//this code suppress a warning we don't want but doesn't do anything
	if( byte & 0 ){ byte = 0; }

	ssp_local[port].handler.callback = dop->handler.callback;
	ssp_local[port].handler.context = dop->handler.context;

	//fill the TX buffer
	ssp_fill_tx_fifo(port, regs);

	//! \todo Use DMA for SSP ssp_local??
	regs->IMSC |= (SSPIMSC_RXIM|SSPIMSC_RTIM); //when RX is half full or a timeout, get the bytes
	ssp_local[port].ret = size;

	return 0;
}

#endif

