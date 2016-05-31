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

static int ssp_port_transfer(int port, int is_read, device_transfer_t * dop);
static void ssp_fill_tx_fifo(int port);
static void ssp_empty_rx_fifo(int port);
static int byte_swap(int port, int byte);

static LPC_SSP_Type * const ssp_regs_table[MCU_SSP_PORTS] = MCU_SSP_REGS;
static u8 const ssp_irqs[MCU_SSP_PORTS] = MCU_SSP_IRQS;

#ifdef LPCXX7X_8X
static void enable_pin(int pio_port, int pio_pin) MCU_PRIV_CODE;
void enable_pin(int pio_port, int pio_pin){
	pio_attr_t pattr;
	pattr.mask = (1<<pio_pin);
	pattr.mode = PIO_MODE_OUTPUT;
	mcu_pio_setattr(pio_port, &pattr);
}
#endif

void _mcu_ssp_dev_power_on(int port){
	if ( ssp_local[port].ref_count == 0 ){

		_mcu_core_priv_enable_irq((void*)(u32)(ssp_irqs[port]));

		switch(port){
		case 0:
#if defined __lpc13uxx || __lpc13xx
			LPC_SYSCON->SSP0CLKDIV = 1;
			LPC_SYSCON->PRESETCTRL |= (1<<0);
#endif
			_mcu_lpc_core_enable_pwr(PCSSP0);
			break;

		case 1:
#if defined __lpc13uxx || __lpc13xx
			LPC_SYSCON->SSP1CLKDIV = 1;
			LPC_SYSCON->PRESETCTRL |= (1<<2);
#endif
			_mcu_lpc_core_enable_pwr(PCSSP1);
			break;

#if MCU_SSP_PORTS > 2
		case 2:
			_mcu_lpc_core_enable_pwr(PCSSP2);
			break;
#endif
		}
		ssp_local[port].duplex_mem = NULL;
		ssp_local[port].handler.callback = NULL;
	}
	ssp_local[port].ref_count++;

}

void _mcu_ssp_dev_power_off(int port){
	if ( ssp_local[port].ref_count > 0 ){
		if ( ssp_local[port].ref_count == 1 ){

			_mcu_core_priv_disable_irq((void*)(u32)(ssp_irqs[port]));

			switch(port){
			case 0:
#if defined __lpc13uxx || __lpc13xx
				LPC_SYSCON->SSP0CLKDIV = 0;
				LPC_SYSCON->PRESETCTRL &= ~(1<<0);
#endif
				_mcu_lpc_core_disable_pwr(PCSSP0);
				break;
			case 1:
#if defined __lpc13uxx || __lpc13xx
				LPC_SYSCON->SSP0CLKDIV = 0;
				LPC_SYSCON->PRESETCTRL &= ~(1<<2);
#endif
				_mcu_lpc_core_disable_pwr(PCSSP1);
				break;
#ifdef LPCXX7X_8X
			case 2:
				_mcu_lpc_core_disable_pwr(PCSSP2);
				break;
#endif
			}
		}
		ssp_local[port].ref_count--;
	}
}

int _mcu_ssp_dev_powered_on(int port){
	return ( ssp_local[port].ref_count != 0 );
}


int mcu_ssp_getattr(int port, void * ctl){
	LPC_SSP_Type * regs;
	spi_attr_t * ctlp = (spi_attr_t*)ctl;

	uint8_t tmp;

	ctlp->pin_assign = ssp_local[port].pin_assign;

	regs = ssp_regs_table[port];

	//Master
	if ( regs->CR1 & (1<<2) ){
		//Slave
		ctlp->master = SPI_ATTR_SLAVE;
	} else {
		ctlp->master = SPI_ATTR_MASTER;
	}

	//width
	ctlp->width = (regs->CR0 & 0x0F) + 1;

	//format
	tmp = (regs->CR0 >> 4) & 0x03;
	if ( tmp == 0 ){
		ctlp->format = SPI_ATTR_FORMAT_SPI;
	} else if ( tmp == 1 ){
		ctlp->format = SPI_ATTR_FORMAT_TI;
	} else if ( tmp == 2 ){
		ctlp->format = SPI_ATTR_FORMAT_MICROWIRE;
	}

	//Mode
	ctlp->mode = 0;
	if ( regs->CR0 & (1<<6) ){
		ctlp->mode |= 0x02;
	}
	if ( regs->CR0 & (1<<7) ){
		ctlp->mode |= 0x01;
	}

	//bitrate
	if ( regs->CPSR != 0){
		ctlp->bitrate = mcu_board_config.core_periph_freq / (regs->CPSR);
	} else {
		ctlp->bitrate = -1;
	}


	return 0;
}

int mcu_ssp_setattr(int port, void * ctl){
	LPC_SSP_Type * regs;
	uint32_t cr0, cr1, cpsr;
	uint32_t tmp;
	spi_attr_t * ctlp = (spi_attr_t*)ctl;

	regs = ssp_regs_table[port];

	if ( ctlp->bitrate == 0 ) {
		errno = EINVAL;
		return -1 - offsetof(spi_attr_t, bitrate);
	}

	if ( ctlp->mode >= 4 ) {
		errno = EINVAL;
		return -1 - offsetof(spi_attr_t, mode);
	}

	if ( ctlp->master != SPI_ATTR_MASTER ){
		errno = EINVAL;
		return -1 - offsetof(spi_attr_t, master);
	}

	if ( ctlp->width < 4 && ctlp->width > 16 ){
		errno = EINVAL;
		return -1 - offsetof(spi_attr_t, width);
	}


	cr0 = 0;
	cr1 = (1<<1); //set the enable

	tmp = mcu_board_config.core_periph_freq / ctlp->bitrate;
	tmp = ( tmp > 255 ) ? 254 : tmp;
	tmp = ( tmp < 2 ) ? 2 : tmp;
	if ( tmp & 0x01 ){
		tmp++; //round the divisor up so that actual is less than the target
	}
	cpsr = tmp;

	if ( ctlp->mode & 0x01 ){
		cr0 |= (1<<7);
	}
	if ( ctlp->mode & 0x02 ){
		cr0 |= (1<<6);
	}

	cr0 |= ( ctlp->width - 1);


	if ( ctlp->format == SPI_ATTR_FORMAT_SPI ){

	} else if ( ctlp->format == SPI_ATTR_FORMAT_TI ){
		cr0 |= (1<<4);
	} else if ( ctlp->format == SPI_ATTR_FORMAT_MICROWIRE ){
		cr0 |= (1<<5);
	} else {
		errno = EINVAL;
		return -1 - offsetof(spi_attr_t, format);
	}

	if ( ctlp->pin_assign != MCU_GPIO_CFG_USER ){

		switch(port){
		case 0:
			if (1){
				switch(ctlp->pin_assign){
				case 0:
					if ( _mcu_ssp_cfg_pio(0,
							MCU_SPI_PORT0_PINASSIGN0,
							MCU_SPI_MOSIPIN0_PINASSIGN0,
							MCU_SPI_MISOPIN0_PINASSIGN0,
							MCU_SPI_SCKPIN0_PINASSIGN0
					) < 0 ){
						errno = ENODEV;
						return -1;
					}
#if defined __lpc13xx
					LPC_IOCON->SCK_LOC = 0;
#endif
					break;
				case 1:
					if( _mcu_ssp_cfg_pio(0,
							MCU_SPI_PORT0_PINASSIGN1,
							MCU_SPI_MOSIPIN0_PINASSIGN1,
							MCU_SPI_MISOPIN0_PINASSIGN1,
							MCU_SPI_SCKPIN0_PINASSIGN1
					) < 0 ){
						errno = ENODEV;
						return -1;
					}

#if defined __lpc13xx
					LPC_IOCON->SCK_LOC = 2;
#endif
					break;
#ifdef LPCXX7X_8X
				case 2:
					if( _mcu_ssp_cfg_pio(0, 2, 27, 26, 22) < 0 ){
						errno = ENODEV;
						return -1;
					}
					break;
#endif
				default:
					errno = EINVAL;
					return -1 - offsetof(spi_attr_t, pin_assign);
				}
			}
			break;
		case 1:
			if (1){
				switch(ctlp->pin_assign){
				case 0:
#ifdef LPCXX7X_8X
					enable_pin(0,9);
					enable_pin(0,8);
					enable_pin(0,7);
#endif
					if ( _mcu_ssp_cfg_pio(1, 0, 9, 8, 7) < 0 ){
						errno = ENODEV;
						return -1;
					}
					break;
#ifdef LPCXX7X_8X
				case 1:
					if ( _mcu_ssp_cfg_pio(1, 4, 23, 22, 20) < 0 ){
						errno = ENODEV;
						return -1;
					}
					break;
#endif
				default:
					errno = EINVAL;
					return -1 - offsetof(spi_attr_t, pin_assign);
				}
			}
			break;
#ifdef LPCXX7X_8X
		case 2:
			if (1){
				switch(ctlp->pin_assign){
				case 0:
					if ( _mcu_ssp_cfg_pio(2, 1, 4, 1, 0) < 0 ){
						errno = ENODEV;
						return -1;
					}
					break;
				default:
					errno = EINVAL;
					return -1 - offsetof(spi_attr_t, pin_assign);
				}
			}
#endif
		}
	}

	regs->CR0 = cr0;
	regs->CR1 = cr1;
	regs->CPSR = cpsr;
	regs->IMSC = 0;

	ssp_local[port].pin_assign = ctlp->pin_assign;

	return 0;
}

int mcu_ssp_swap(int port, void * ctl){
	return byte_swap(port, (int)ctl);
}

int mcu_ssp_setduplex(int port, void * ctl){
	ssp_local[port].duplex_mem = (void * volatile)ctl;
	return 0;
}

static void exec_callback(int port, LPC_SSP_Type * regs, void * data){
	regs->IMSC &= ~(SSPIMSC_RXIM|SSPIMSC_RTIM); //Kill the interrupts
	if ( ssp_local[port].size == 0 ){
		_mcu_core_exec_event_handler(&(ssp_local[port].handler), (mcu_event_t)0);
	}
}


int mcu_ssp_setaction(int port, void * ctl){
	mcu_action_t * action = (mcu_action_t*)ctl;
	LPC_SSP_Type * regs;
	regs = ssp_regs_table[port];

	if( action->callback == 0 ){
		if ( regs->IMSC & (SSPIMSC_RXIM|SSPIMSC_RTIM) ){
			exec_callback(port, regs, MCU_EVENT_SET_CODE(MCU_EVENT_OP_CANCELLED));
		}
		ssp_local[port].handler.callback = 0;
		return 0;
	}

	if( _mcu_core_priv_validate_callback(action->callback) < 0 ){
		return -1;
	}

	ssp_local[port].handler.callback = action->callback;
	ssp_local[port].handler.context = action->context;

	_mcu_core_setirqprio(ssp_irqs[port], action->prio);


	return 0;
}


int byte_swap(int port, int byte){
	LPC_SSP_Type * regs;
	regs = ssp_regs_table[port];
	regs->DR = byte;
	while ( regs->SR & SSPSR_BSY ){
		;
	}
	byte = regs->DR; //read the byte to empty the RX FIFO
	return byte;

}

int _mcu_ssp_dev_write(const device_cfg_t * cfg, device_transfer_t * wop){
	int port = DEVICE_GET_PORT(cfg);
	return ssp_port_transfer(port, 0, wop);
}

int _mcu_ssp_dev_read(const device_cfg_t * cfg, device_transfer_t * rop){
	int port = DEVICE_GET_PORT(cfg);
	return ssp_port_transfer(port, 1, rop);
}

void ssp_fill_tx_fifo(int port){
	LPC_SSP_Type * regs;
	regs = ssp_regs_table[port];
	int size = 0;
	while ( (regs->SR & SSPSR_TNF) && ssp_local[port].size && (size < 8) ){
		if ( ssp_local[port].tx_buf != NULL ){
			//! \todo This won't handle spi widths other than 8 bits -- need to read the transmit width
			regs->DR = *ssp_local[port].tx_buf++;
		} else {
			regs->DR = 0xFF;
		}
		ssp_local[port].size--;
		size++;
	}
}

void ssp_empty_rx_fifo(int port){
	LPC_SSP_Type * regs;
	regs = ssp_regs_table[port];
	while ( regs->SR & SSPSR_RNE ){
		if ( ssp_local[port].rx_buf != NULL ){
			*ssp_local[port].rx_buf++ = regs->DR;
		} else {
			regs->DR;
		}
	}
}

void _mcu_core_ssp_isr(int port){
	LPC_SSP_Type * regs;
	regs = ssp_regs_table[port];
	ssp_empty_rx_fifo(port);
	ssp_fill_tx_fifo(port);
	if ( (regs->SR & (SSPSR_TFE)) && !(regs->SR & (SSPSR_RNE)) ){
		exec_callback(port, regs, 0);
	}
}

void _mcu_core_ssp0_isr(){
	_mcu_core_ssp_isr(0);
}

void _mcu_core_ssp1_isr(){
	_mcu_core_ssp_isr(1);
}

void _mcu_core_ssp2_isr(){
	_mcu_core_ssp_isr(2);
}

int ssp_port_transfer(int port, int is_read, device_transfer_t * dop){
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

	if( _mcu_core_priv_validate_callback(dop->callback) < 0 ){
		return -1;
	}

	ssp_local[port].handler.callback = dop->callback;
	ssp_local[port].handler.context = dop->context;

	ssp_fill_tx_fifo(port);

	//! \todo Use DMA for SSP ssp_local??
	regs->IMSC |= (SSPIMSC_RXIM|SSPIMSC_RTIM); //when RX is half full or a timeout, get the bytes
	ssp_local[port].ret = size;

	return 0;
}

#endif

